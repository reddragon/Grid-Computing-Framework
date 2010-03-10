#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include <set>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>
#include <cmath>

#include "parser.h"
#include "config.h"
#include "archiver.h"

using namespace std;

typedef struct node
{
	long long time_stamp;
	int busy;
	string node_address;
	double network_metric, performance_metric;
	
	bool operator > (const node &n)
	{
		return node_address > n.node_address;
	}
} node;

set <node> node_set;
map <string, node> node_map;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void * client_cleanup(void * args)
{
	while(1)
	{
		usleep(AUTOCLEANUPTIME);
		long long cur_time = time(NULL);
		vector <string> dead_nodes;
		
		for(map<string,node>::iterator it = node_map.begin(); it != node_map.end(); it++)

		{
		   if( (cur_time - it->second.time_stamp) >= CLIENTTIMEOUT)
			   dead_nodes.push_back(it->first);	
		}

		for(int i = 0; i < dead_nodes.size(); i++)
		{
			//cout << dead_nodes[i] << " has died" << endl;
			node_map.erase(dead_nodes[i]);
		}

	}
}

struct fileinfo
{
	string file_name, host_name;
	int file_size;
};

int get_file_size(string file_name)
{
	FILE *fp;
	fp = fopen(file_name.c_str(), "rb");
	if(fp == NULL)
	{
		printf("Error in reading file in get_file_size().");
		exit(1);
	}
	int lsize = 0;
	fseek(fp,0,SEEK_END);
     	lsize=ftell(fp);
	return lsize;
}

int sf_sockfd;
struct sockaddr_in sf_serv_addr;
bool sf_sock_bound;
void * send_file(void * args)
{
     
     cout << "Ready to send\n" << endl;
     fileinfo *fi = (fileinfo *)args;
     string file_name = fi->file_name, host_name = fi->host_name;
     int newsockfd, portno;
     int &sockfd = sf_sockfd;
     socklen_t clilen;
     char *buffer;
     struct sockaddr_in cli_addr;
     struct sockaddr_in &serv_addr = sf_serv_addr;	bool command_sent = false;  
     long lsize;
     fflush(stdout);
     FILE *fp;
     fp=fopen(file_name.c_str(),"rb");
     if (fp==NULL) 
     { 
		printf("Error in reading file in send_file().\n"); 
		exit (1);
     }
	 
     int n;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
     { 
        printf("Error in opening socket");
    	exit(0); 
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(atoi(PORT5));
     int optval;
     if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
     {
	   fprintf(stderr, "Failed to set socket options\n");
	   exit(1);
     }
     
     
     //if(!sf_sock_bound)
     {
	    sf_sock_bound = true;
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
     { 
              printf("Error in binding socket in send_file()");
	      exit(1);
     }
     }
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
     {
          printf("Error in accepting connection in send_file()");
	  return NULL;
     }

     fseek(fp,0,SEEK_END);
     lsize=ftell(fp);
     rewind(fp);
	 
     buffer=(char *)malloc(sizeof(char)*lsize); 
     fread(buffer,1,lsize,fp);
     fclose(fp);
     fflush(stdout);
     cout << "Uptil Here";
     n = write(newsockfd,buffer,lsize);
     if (n < 0) 
     {	     
	     printf("Error in writing to socket");
	     return NULL;
     }
     
     close(sockfd);
     return NULL; 
}

void * receive_file(void * args)
{
    fileinfo * f = (fileinfo *) args;
    string file_name = f->file_name, host_name = f->host_name;
    int sockfd, portno;
	long n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
		
    char buffer[MAXSIZE];
    FILE *fp;
    fp=fopen(file_name.c_str(),"wb");
	
    portno = atoi(PORT6);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        printf("Error in opening socket");
	exit(1);
    }
    server = gethostbyname(host_name.c_str());
    if (server == NULL) {
        fprintf(stderr,"Error, no such host\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        printf("Error in connecting");
	return NULL;
    }
	
   int to_receive = f->file_size;
   while(to_receive > 0)
   {
		n = read(sockfd,buffer,sizeof(buffer));
		if(n <= 0) break;

		to_receive -= n;
		fwrite(buffer,1,n,fp);
   }
   fclose(fp); 
   return NULL;
}

struct command
{
	string command_str, client;
};

void send_command_to_client(command arg)
{
	int sockfd;
        struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, optval = 1;
	char messagebuffer[500];
	
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(arg.client.c_str(),PORT3, &hints, &servinfo)) != 0) 
	{
               fprintf(stderr, "Error in send_command_to_client: %s\n", gai_strerror(rv));
               exit(1);
        }

        for(p = servinfo; p != NULL; p = p->ai_next) 
	{
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        	{
                    perror("Error in send_command_to_client(): Could not create socket\n");
                     continue;
                 }
		break;
       }
	
        if (p == NULL) 
	{
                 fprintf(stderr, "Error in send_command_to_client(): Failed to bind socket\n");
                 exit(1);
        }	
	
	strcpy(messagebuffer, arg.command_str.c_str());	
	
	if ((numbytes = sendto(sockfd, messagebuffer, 
			strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) 
	{
			perror("Error in send_command_to_client(): Message could not be sent\n");
			exit(1);
	}
	
	freeaddrinfo(servinfo);
	close(sockfd);

}

set< pair<string,string> > client_response;

void * wait_for_client_response_to_command( void * args )
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char tbuf[MAXBUFLEN];
    
    int rv, numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    size_t addr_len;
    char s[INET6_ADDRSTRLEN], contacting_client[100];
    fd_set readfds, masterfds;
    struct timeval timeout;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PORT4, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Error in wait_for_client_response_to_command(): %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error in wait_for_client_response_to_command(): Could not create socket\n");
            continue;
        }
    int optval;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
	   fprintf(stderr, "Failed to set socket options\n");
	   exit(1);
    }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
            close(sockfd);
            perror("Error in wait_for_client_response_to_command(): Failed to bind socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "Error in wait_for_client_response_to_command(): Failed to bind socket\n\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    addr_len = sizeof their_addr;
   
    while(1)
   {
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) 
	  	 {
			perror("recvfrom");
			exit(1);
       		}
	   	buf[numbytes] = '\0';

		strcpy(contacting_client, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		
		string response = (string)buf;
		cout << contacting_client << " responded with " << response << endl;
  		client_response.insert(make_pair(contacting_client,response));
  }
	freeaddrinfo(servinfo);
	close(sockfd);

}

fileinfo fi;


void * command_coordinator_stub(void * args)
{
	pthread_t ccresponse_thread;
	pthread_create(&ccresponse_thread, NULL, &(wait_for_client_response_to_command), NULL);
	
	string file_name = "t1.tar.gz";
	string file_send_command = "COLLECTFILE";
	int file_size = get_file_size(file_name);
	command c, filecoord;
	stringstream ss;
	ss << file_send_command << " " << file_name << " " << file_size; 
	c.command_str = ss.str();

	bool command_sent = false;  
	string recipient_client = "";
	int wait = 0;
	string desired_response = "COLLECTFILEACK";
	pthread_t sender_thread;
	fi.file_name = file_name;	
	pthread_create(&sender_thread, NULL, &(send_file), &fi);

	
	bool received_reply = false;
	while(1)
	{
		usleep(1000000);
		for(map<string,node>::iterator it = node_map.begin(); it != node_map.end() && command_sent == false; it++)
		{
			c.client = it->first;
			send_command_to_client(c);
			recipient_client = it->first;
			command_sent = true;
		}
		if(command_sent == true) break;
	}
	for(int i = 0; i < 5; i++)
	{
		if(client_response.find(make_pair(recipient_client,desired_response)) != client_response.end())
		{
			received_reply = true;
			cout << "Received reply from " << recipient_client << endl;
			client_response.erase(make_pair(recipient_client,desired_response));
			break;
		}
		usleep(500000);
	}
	
	int network_delay = 5, problem_timeout = 20;
	if(!received_reply) cout << "Did not receive any reply" << endl;
	else
	{
		int timeout = (network_delay + problem_timeout) * 1000000 + 500000;
		int checks = (int)ceil(timeout*1.0/100000);
		bool op_recd = false;
		
		for(int i = 0; i < checks && !op_recd; i++)
		{
			string arg0, arg1;
			for(set< pair<string,string> >::iterator it = client_response.begin(); it != client_response.end(); it++)
			{
				if(it->first != recipient_client) continue;
				stringstream sst(it->second);
				sst >> arg0;
				
				if(arg0 == "COLLECTRES")
				{
				   int file_size;
				   sst >> arg1 >> file_size;
					fileinfo rfi;
					rfi.file_name = arg1; rfi.host_name = recipient_client;
					rfi.file_size = file_size; 
					receive_file(&rfi);
					command collresack;
					collresack.command_str = "COLLECTRESACK " + arg1;
					collresack.client = recipient_client;
					send_command_to_client(collresack);
					cout << "Yo baby!" << endl;
					op_recd = true;
					break;
				}
			}
			usleep(100000);
		} 
	}
	
	pthread_join(sender_thread,NULL);
}


void * listen_to_client_ping(void * args)
{ 
    int sockfd, i;
    struct addrinfo hints, *servinfo, *p;
    
    int * result;
    result =  new int;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    size_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char contacting_client[100];
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, PORT2, &hints, &servinfo)) != 0) {
        fprintf(stderr, "Error in listen_to_client_ping(): %s\n", gai_strerror(rv));
        exit(1); 
    }

    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
            perror("Error in listen_to_client_ping(): Could not create socket\n");
            continue;
        }
       
       	int optval;
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
	   fprintf(stderr, "Failed to set socket options\n");
	   exit(1);
	}
	//TODO
           if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Error in listen_to_client_ping(): Failed to bind socket\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Error in listen_to_client_ping(): Failed to bind socket\n\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    pthread_t cleanup_thread;
    pthread_create(&cleanup_thread, NULL, &(client_cleanup), NULL);
    long long time_stamp = 0;
    while(1)
    {

		time_stamp = (time_stamp + 1);
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) 
	  	 {
			perror("recvfrom");
			exit(1);
       		}
	   	buf[numbytes] = '\0';

		strcpy(contacting_client, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		
		string client = (string)contacting_client;
		double nw_metric, pf_metric;
		char message[100];	
		stringstream ss((string)buf);
		ss >> message >> nw_metric >> pf_metric;
		
		if(strcmp(message, "CLIRESP") == 0)
		{
			if(clock() == -1)
			{
				printf("Clock error in listen_to_client_ping()\n");
				exit(1);
			}
			
				bool command_sent = false;  
			if(node_map.find(client) != node_map.end())
			{
				node_map[client].time_stamp = time(NULL);
				node_map[client].network_metric = nw_metric;
				node_map[client].performance_metric = pf_metric;
			}
			else
			{
				node client_node;
				client_node.time_stamp = time(NULL);
				client_node.network_metric = nw_metric;
				client_node.performance_metric = pf_metric;
				client_node.busy = -1;
				node_map[client] = client_node;
			}
		}	
		
	
    }  
    close(sockfd);

 	
}

void * broadcast_ping(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, numbytes, optval = 1;
    char messagebuffer[500];
    char hostname[500];
    
    strcpy(hostname, "255.255.255.255");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
	

    if ((rv = getaddrinfo(hostname,PORT1, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "Error in broadcast_ping(): %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
            perror("Error in broadcast_ping(): Could not create socket\n");
            continue;
        }

        break;
    }

    if (p == NULL) 
	{
        fprintf(stderr, "Error in broadcast_ping(): Failed to bind socket\n");
        exit(1);
    }
	
	/* Required to permit UDP Broadcasts */
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, 4) == -1)
	{
		fprintf(stderr, "Error in broadcast_ping(): Failed to set socket options\n");
		exit(1);
	}
	
	/* SERVERADDREQ message */
	strcpy(messagebuffer, "SERVERPING");
    
   while(1)
   { 
    if ((numbytes = sendto(sockfd, messagebuffer, strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("Error in broadcast_ping(): Message could not be sent\n");
        exit(1);
    }
    //printf("Pinged\n");
    usleep(SERVERPINGGAP);
   }

}

XMLFile *xf;
ParsedXMLElements *pxe;

//vector< vector<int> > task_sorted_list;
map<int,int> task_status;
int total_tasks_done;

/*
void do_topo_sort(ParsedXMLElements *pxe)
{
	//TODO
	//Do Topo Sort
	
	//This is a dummy sort
	vector<int> r;
	for(int i = 0; i < (pxe->Tasks.size()); i++)
	{
		r.push_back(i);
		
	}
	task_sorted_list.push_back(r);	
}*/

void init_task_status()
{
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		task_status[i] = TO_BE_DONE;
}

int get_next_task()
{	
	bool processing_tasks = false;
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		if(task_status[i] == TO_BE_DONE)
			return i;
		else if(task_status[i] == PROCESSING)
			processing_tasks = true;
  
	/*
		//TODO
		If no task can be executed yet,
		return -2

		If no task is available yet, because 
		they are being executed,
		return -3

	*/
	
	if(processing_tasks == true) 
		return -3;

	return -1;
		
}

string load_balancer(int i)
{
	//TODO
	//Load Balancing Algorithm goes here
	for(map<string,node>::iterator it = node_map.begin(); it != node_map.end(); it++)
	{
		if((it->second).busy == -1)
			return it->first;	
	}

	return "";
}

struct coordinate_task_args
{
	string selected_client;
	int task;
};

void * coordinate_task(void *arg)
{
	coordinate_task_args *cta = (coordinate_task_args *) arg;
	int task = (cta)->task;
	
	string file_name = pxe->Tasks[task].task_id + ".tar.gz";
	string file_send_command = "COLLECTFILE";
	int file_size = get_file_size(file_name);
	command c, filecoord;
	stringstream ss;
	ss << file_send_command << " " << file_name << " " << file_size; 
	c.command_str = ss.str();

	cout << "here" << endl;
	bool command_sent = false;  
	string recipient_client = (cta)->selected_client;
	
	int wait = 0;
	string desired_response = "COLLECTFILEACK";
	pthread_t sender_thread;
	fi.file_name = file_name;	
	pthread_create(&sender_thread, NULL, &(send_file), &fi);
	
	bool received_reply = false;
	c.client = recipient_client;
	send_command_to_client(c);
	command_sent = true;

	for(int i = 0; i < CFRRETRIES; i++)
	{
		if(client_response.find(make_pair(recipient_client,desired_response)) != client_response.end())
		{
			received_reply = true;
			cout << "Received reply from " << recipient_client << endl;
			client_response.erase(make_pair(recipient_client,desired_response));
			break;
		}
		usleep(CFRTIMEOUT);
	}
	int network_delay = pxe->Tasks[task].network_latency_time;
	int problem_timeout = (pxe->Tasks[task].timeout);
	if(!received_reply) 
	{	
		cout << "Did not receive any reply from " << recipient_client << " to solve " << pxe->Tasks[task].task_id << endl;
		node_map[cta->selected_client].busy = -1;
		task_status[cta->task] = TO_BE_DONE;

		return NULL;
	}
	else
	{
		int timeout = (network_delay + problem_timeout) * 1000000 + 500000;
		int checks = (int)ceil(timeout*1.0/100000);
		bool op_recd = false;
		
		for(int i = 0; i < checks && !op_recd; i++)
		{
			string arg0, arg1;
			for(set< pair<string,string> >::iterator it = client_response.begin(); it != client_response.end(); it++)
			{
				if(it->first != recipient_client) continue;
				stringstream sst(it->second);
				sst >> arg0;
				
				if(arg0 == "COLLECTRES")
				{
				   int file_size;
				   sst >> arg1 >> file_size;
					fileinfo rfi;
					rfi.file_name = arg1; rfi.host_name = recipient_client;
					rfi.file_size = file_size; 
					receive_file(&rfi);
					command collresack;
					collresack.command_str = "COLLECTRESACK " + arg1;
					collresack.client = recipient_client;
					send_command_to_client(collresack);
					cout << "Yo baby!" << endl;
					op_recd = true;
					break;
				}
			}
			usleep(100000);
		} 
	}
	
	pthread_join(sender_thread,NULL);
	
	
	usleep(1000000);
	node_map[cta->selected_client].busy = -1;
	task_status[cta->task] = DONE;
	return NULL;
}

void *start(void *args)
{
	total_tasks_done = 0;
	string pss_file = "inp.in";
	xf = new XMLFile;
	if(xf == NULL) 
	{
		fprintf(stderr, "Failed to allocate memory");
		exit(1);
	}

	pxe = new ParsedXMLElements;
	if(pxe == NULL)
	{
		fprintf(stderr, "Failed to allocate memory");
		exit(1);
	}

	int parse_error = start_to_parse(xf, pxe, pss_file);	     
	if(parse_error)
	{
		fprintf(stderr, "Problem in parsing PSS File. Terminating.\n");
		exit(1);
		
	}
	//do_topo_sort(pxe);
	init_task_status();
	create_task_archives(xf,pxe);
	
	pthread_t ccresponse_thread;
	pthread_create(&ccresponse_thread, NULL, &(wait_for_client_response_to_command), NULL);
	sf_sock_bound = true;
	while(1)
	{
		int next_task = get_next_task();
		//cout << next_task << " here " << endl;
		if(next_task == -2 || next_task == -3)
		{
			do
			{
				usleep(500000);
				next_task = get_next_task();
			} while(next_task == -2 || next_task == -3);
		}
		
		if(next_task == -1)
		{	
			
			printf("Execution completed.\n");
			break;
		}

			
		string selected_client = "";
		while(1)
		{
			selected_client = load_balancer(next_task);
			if(selected_client != "") break;
			usleep(500000);
		} 
		
		cout << next_task << endl;	
		coordinate_task_args *cta;
		cta = new coordinate_task_args;
		cta->selected_client = selected_client;
		cta->task = next_task;
		task_status[next_task] = PROCESSING;
		node_map[selected_client].busy = 1;
		pthread_t cta_thread;
		pthread_create(&cta_thread, NULL, &(coordinate_task), cta);
		
	}
	
	//pthread_join(ccresponse_thread, NULL);
	delete xf;
	delete pxe;
	return NULL;
}

int main()
{
	pthread_t ping_thread, listen_to_client_ping_thread, ccstub_thread, start_thread;
	pthread_create(&ping_thread, NULL, &(broadcast_ping), NULL);
	pthread_create(&listen_to_client_ping_thread, NULL , &(listen_to_client_ping), NULL);
	
	//pthread_create(&ccstub_thread, NULL, &(command_coordinator_stub), NULL);
	//pthread_join(ccstub_thread, NULL);
	
	pthread_create(&start_thread, NULL, &(start), NULL);
	pthread_join(start_thread,NULL);
	//pthread_join(listen_to_client_ping_thread, NULL);
	//pthread_join(ping_thread, NULL);
	return 0;
}
