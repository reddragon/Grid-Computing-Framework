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
#include <pthread.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <sstream>
#include "task_ce.h"

#define PORT1 "6366" 
#define PORT2 "6367"
#define PORT3 "6368"
#define PORT4 "6369"
#define PORT5 "6370"
#define PORT6 "6371"
#define PORT7 "6372"
#define CONNECTIONRETRIES 10
#define RETRYGAP 200000

#define SERVERTIMEOUTSEC 4
#define SERVERTIMEOUTUSEC 0
#define MAXBUFLEN 1000
#define PERFMETRICITERATIONS 500000
#define RECALCULATIONTIMEGAP 30000000
#define CLIENTRESPONSETIMEGAP 200000
#define MAXSIZE 10000000

using namespace std;

char server_address[100];
char buf[MAXBUFLEN];
double nw_metric, pf_metric;

struct nw_metric_params
{
        int packet_size, packet_count; 
        char *server_add;
};



void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);
   
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

double performance_metric()
{

	int start = clock(), i, j, k, l;
	double p=1.231;
	
	int mat1[3][3], mat2[3][3];
	mat1[0][0] = 1; mat1[0][1] = 4; mat1[0][2] = 9;
	mat1[1][0] = 2; mat1[1][1] = 5; mat1[1][2] = 8;
	mat1[2][0] = 3; mat1[2][1] = 6; mat1[2][2] = 7;
	
	mat2[0][0] = 9; mat2[0][1] = 5; mat2[0][2] = 4;
	mat2[1][0] = 2; mat2[1][1] = 1; mat2[1][2] = 5;
	mat2[2][0] = 4; mat2[2][1] = 4; mat2[2][2] = 6;
	
	for(i = 1; i < PERFMETRICITERATIONS; i++)
	{
		p = p * p * pow(p,i);
		p = p / (p * 0.912);
		
		int mat[3][3];
		
		for(j = 0; j < 3; j++)
			for(k = 0; k < 3; k++)
			{
				int temp = 0;
				for(l = 0; l < 3; l++)
					temp += mat1[j][l] * mat2[l][k];
				
				mat[j][k] = temp;
			}
		
		for(j = 0; j < 3; j++)
			for(k = 0; k < 3; k++)
				mat1[j][k] = (mat[j][k] % 100007);
		
	}
	int end = clock();
	
	double t = (end - start) * 1.0 / CLOCKS_PER_SEC;
	t = 1 / t;
	return t;
}

double network_metric(int packet_size, int packet_count, char *server_add )
{
	int temp_garb;
	char reliability_buf[100], latency_buf[100], tempbuf[100];
	char command[100];
	int packets_lost, reliability;
	double min_latency, avg_latency, max_latency, mdev_latency;
	double network_metric_value = 0;
	FILE *pingreader;
	
	sprintf(command, "ping -q -s %d -c %d %s > pingstats.tmp", 
			packet_size, packet_count, server_add);
	//printf("Command is : %s \n", command);
	
	system(command);
	pingreader = fopen("pingstats.tmp", "r");
	fseek(pingreader, 0, SEEK_SET);
	
	/*
		Strictly depends upon the following format:
		PING 127.0.0.1 (127.0.0.1) 20(48) bytes of data.

		--- 127.0.0.1 ping statistics ---
		1 packets transmitted, 1 received, 0% packet loss, time 0ms
		rtt min/avg/max/mdev = 0.030/0.030/0.030/0.000 ms
	 */
	
	fgets(tempbuf, 100, pingreader);
	fgets(tempbuf, 100, pingreader);
	fgets(tempbuf, 100, pingreader);
	fgets(reliability_buf, 100, pingreader);
	fgets(latency_buf, 100, pingreader);
	
	sscanf(reliability_buf, "%d packets transmitted, %d received, \
			%d%% packet loss, time %dms", &temp_garb, &temp_garb, 
			&packets_lost, &temp_garb);
	sscanf(latency_buf, "rtt min/avg/max/mdev = %lf/%lf/%lf/%lf ms", 
			&min_latency, &avg_latency, &max_latency, &mdev_latency);
	
	/*
			Network Metric Computation
			n = reliability * 1 / ( avg_latency + 2 * mdev_latency)
	*/
	
	reliability = 100 - packets_lost;
	//printf("Reliability: %d\tAverage Latency: %lf\tMean Dev: %lf\n", reliability, avg_latency, mdev_latency);
	
	network_metric_value = reliability * 1.0 /((avg_latency + 2 * mdev_latency) * 1.0);
	return network_metric_value;
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


void * send_file(void * args)
{
     fileinfo *fi = (fileinfo *)args;
     string file_name = fi->file_name, host_name = fi->host_name;
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char *buffer;
     struct sockaddr_in serv_addr, cli_addr;
     long lsize;
     fflush(stdout);
     FILE *fp;
     fp=fopen(file_name.c_str(),"rb");
     if (fp==NULL) 
     { 
		printf("Error in reading file in send_file()."); 
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
     serv_addr.sin_port = htons(atoi(PORT6));
     int optval;
     if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
     {
	   fprintf(stderr, "Failed to set socket options\n");
	   exit(1);
    }

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
     { 
              printf("Error in binding socket in send_file()");
	      exit(1);
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

     lsize = fi->file_size;
     	 
     buffer=(char *)malloc(sizeof(char)*lsize); 
     fread(buffer,1,lsize,fp);
     fclose(fp);
     fflush(stdout);
     n = write(newsockfd,buffer,lsize);
     if (n < 0) 
     {	     
	     printf("Error in writing to socket");
	     return NULL;
     }
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
    /*
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }*/
    FILE *fp;
    fp=fopen(file_name.c_str(),"wb");
	
    portno = atoi(PORT5);
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
    int retries = 0;
    
   if( connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
    {
	    cout << "Error in connecting!\n";
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

int get_server_address()
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    
    int rv, numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    size_t addr_len;
    char s[INET6_ADDRSTRLEN];
    fd_set readfds, masterfds;
    struct timeval timeout;
    int connretriesleft = CONNECTIONRETRIES;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Error in get_server_address(): %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error in get_server_address(): Could not create socket\n");
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
            perror("Error: Failed to bind socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "Error in get_server_address(): Failed to bind socket\n\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    printf("Waiting for a message...\n");
	
	addr_len = sizeof their_addr;
	
	while((connretriesleft--) > 0)
	{
		timeout.tv_sec = SERVERTIMEOUTSEC;
		timeout.tv_usec = SERVERTIMEOUTUSEC;
		FD_ZERO(&masterfds);
		FD_SET(sockfd, &masterfds);

		memcpy(&readfds, &masterfds, sizeof(fd_set));

		if (select(sockfd+1, &readfds, NULL, NULL, &timeout) < 0)
		{
			perror("on select");
			exit(1);
		}

		if (FD_ISSET(sockfd, &readfds))
			break;
			
		else
		{
			printf("The connection timed out. Attempting again.\n");
			usleep(RETRYGAP);
			continue;
		} 
	}
	if(connretriesleft <= 0)
	{
		printf("Could not connect with the server. Terminating.\n");
		close(sockfd);
		exit(1);
	}
	
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) 
	{
		perror("recvfrom");
		exit(1);
	}
	buf[numbytes] = '\0';
	strcpy(server_address, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		
	printf("Message from server: %s\n", buf);	
	printf("The server address is: %s\n", server_address);
	close(sockfd);
	return 1;

}

void * check_connection_with_server(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char tbuf[MAXBUFLEN];
    
    int rv, numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    size_t addr_len;
    char s[INET6_ADDRSTRLEN];
    fd_set readfds, masterfds;
    struct timeval timeout;
    int connretriesleft = CONNECTIONRETRIES;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Error in check_connection_with_server(): %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error in check_connection_with_server(): Could not create socket\n");
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
            perror("Error in check_connection_with_server(): Failed to bind socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "Error in check_connection_with_server(): Failed to bind socket\n\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    addr_len = sizeof their_addr;
   
	while((connretriesleft--) > 0)
	{
		timeout.tv_sec = SERVERTIMEOUTSEC;
		timeout.tv_usec = SERVERTIMEOUTUSEC;
		FD_ZERO(&masterfds);
		FD_SET(sockfd, &masterfds);

		memcpy(&readfds, &masterfds, sizeof(fd_set));

		if (select(sockfd+1, &readfds, NULL, NULL, &timeout) < 0)
		{
			perror("on select");
			exit(1);
		}

		if (FD_ISSET(sockfd, &readfds))
		{ 
			 //printf("Got pinged\n");
			 connretriesleft = CONNECTIONRETRIES; 
			 if ((numbytes = recvfrom(sockfd, tbuf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
			{
				perror("recvfrom");
				exit(1);
			}

			 
			 continue; }
			
		else
		{
			printf("The connection timed out. Attempting again.\n");
			usleep(RETRYGAP);
			continue;
		} 
	}
	if(connretriesleft <= 0)
	{
		printf("Could not connect with the server. Terminating.");
		close(sockfd);
		exit(1);
	}
}


void * recalculate_metrics(void *r)
{
        struct nw_metric_params *n = (struct nw_metric_params *)(r);
        
        while(1)
        {
                
                usleep(RECALCULATIONTIMEGAP);
                pf_metric = performance_metric();
                nw_metric = network_metric(n->packet_size, n->packet_count, n->server_add);	
                printf("Metrics recalculated. Performance Metric: %lf  Network Metric: %lf\n", pf_metric, nw_metric);
	}
}


void * ping_the_server(void * args)
{
	int sockfd;
        struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, optval = 1;
	char messagebuffer[500];
	
	printf("Calculating network metric..\n");
	nw_metric = network_metric(64, 2, server_address);
	printf("Network metric calculated.\n");
	printf("Calculating performance metric..\n");
	pf_metric = performance_metric();
	printf("Performance metric calculated.\n");
        printf("Metrics calculated. Performance Metric: %lf  Network Metric: %lf\n", pf_metric, nw_metric);

	
	struct nw_metric_params nmp;
	nmp.packet_size = 64;
	nmp.packet_count = 2;
	nmp.server_add = (char *) malloc(sizeof(char) * 100);
	strcpy(nmp.server_add , server_address);
	
	
	pthread_t thread;
    	pthread_create (&thread, NULL, &(recalculate_metrics), (&nmp));
        //pthread_join(thread,NULL);
	
	
	//start_recalculating_metrics(nmp);
	
	
	
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

	printf("Server address %s \n", server_address);
        if ((rv = getaddrinfo(server_address,PORT2, &hints, &servinfo)) != 0) 
	{
               fprintf(stderr, "Error: %s\n", gai_strerror(rv));
                return NULL;
        }

        for(p = servinfo; p != NULL; p = p->ai_next) 
	{
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        	{
                    perror("Error: Could not create socket\n");
                     continue;
                 }
                 break;
        }
	

        if (p == NULL) 
	{
                 fprintf(stderr, "Error: Failed to bind socket\n");
                 return NULL;
        }
	
	
	
	while(1)
	{	
		usleep(CLIENTRESPONSETIMEGAP);
		sprintf(messagebuffer, "CLIRESP %lf %lf", nw_metric, pf_metric);
		if ((numbytes = sendto(sockfd, messagebuffer, 
			strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) 
		{
			perror("Error: Message could not be sent\n");
			exit(1);
		}
	}
	freeaddrinfo(servinfo);
	close(sockfd);
}

struct command
{
	string command_str, client;
};

fileinfo * fi;

void send_command_to_server(command arg)
{
	int sockfd;
        struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, optval = 1;
	char messagebuffer[500];
	
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(arg.client.c_str(),PORT4, &hints, &servinfo)) != 0) 
	{
               fprintf(stderr, "Error in send_command_to_server(): %s\n", gai_strerror(rv));
               exit(1);
        }

        for(p = servinfo; p != NULL; p = p->ai_next) 
	{
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        	{
                    perror("Error in send_command_to_server(): Could not create socket\n");
                     continue;
                 }
		break;
       }
	
        if (p == NULL) 
	{
                 fprintf(stderr, "Error in send_command_to_server(): Failed to bind socket\n");
                 exit(1);
        }	
	
	strcpy(messagebuffer, arg.command_str.c_str());	
	
	if ((numbytes = sendto(sockfd, messagebuffer, 
			strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) 
	{
			perror("Error in send_command_to_server(): Message could not be sent\n");
			exit(1);
	}
	
	freeaddrinfo(servinfo);
	close(sockfd);


}

struct received_command
{
	string command_received;
	vector<string> args;
} cmd_rcd;



void * receive_commands(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char tbuf[MAXBUFLEN];
    
    int rv, numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    size_t addr_len;
    char s[INET6_ADDRSTRLEN], contacting_mc[100];
    fd_set readfds, masterfds;
    struct timeval timeout;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PORT3, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Error in receive_commands(): %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error in receive_commands(): Could not create socket\n");
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
            perror("Error in receive_commands(): Failed to bind socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "Error in receive_commands(): Failed to bind socket\n\n");
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

		strcpy(contacting_mc, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		if(strcmp(contacting_mc, server_address) == 0)
		{
			cout << "Received the command: " << buf << " from the server" << endl;
			stringstream cm((string)buf);
			string cmd, arg0, arg1;
			cm >> cmd;
				
			if(cmd == "TEST")
			{
				command c;
				c.client = (string)server_address;
				c.command_str = "TESTREPLY";
				send_command_to_server(c);	
			}
			
			if(cmd == "COLLECTRESACK")
			{
				cm >> arg0;
				//cout << "... " << arg0 << endl;
			}
			
			if(cmd == "COLLECTFILE")
			{
				cm >> arg0 >> arg1;
				stringstream ss(arg1);
				int file_size;
				ss >> file_size;
				command c;
				c.client = (string)server_address;
				c.command_str = "COLLECTFILEACK";
				send_command_to_server(c);
				fi = new fileinfo;
				fi->file_name = arg0; fi->host_name = c.client; fi->file_size = file_size;			
				

				if(!receive_file((void *)fi))
				{
					cout << "File " << arg0 << " received successfully." << endl;
					cmd_rcd.command_received = cmd;
					cmd_rcd.args.clear();
					int ce_ret = task_ce(arg0.substr(0,arg0.size()-((string)".tar.gz").size()));				
					command collect_res;
					collect_res.client = (string)server_address;
					stringstream ss;
					string res_file = arg0.substr(0,arg0.size()-((string)".tar.gz").size()) + "_op.tar.gz";
					ss << "COLLECTRES " << res_file << " " << get_file_size(res_file);
					collect_res.command_str = ss.str();
					
					fileinfo rfi;
					rfi.host_name = server_address;
					rfi.file_name = res_file;
					rfi.file_size = get_file_size(res_file);
					
					send_command_to_server(collect_res);
					send_file(&rfi);
				}
				else
					cout << "Error in receiving file " << arg0 << endl;
					
			}

		}
  
  }
	freeaddrinfo(servinfo);
	close(sockfd);

}

void * command_receiver_and_responder_stub(void * args)
{
	pthread_t cmd_receiver_thread;
	pthread_create(&cmd_receiver_thread, NULL, &(receive_commands), NULL);
	pthread_join(cmd_receiver_thread, NULL);
}

int main()
{
	get_server_address();
	pthread_t connection_checker;
	pthread_create(&connection_checker, NULL, &(check_connection_with_server), NULL);
	
	pthread_t ping_server;
	pthread_create(&ping_server, NULL, &(ping_the_server), NULL);
	
	pthread_t crr_stub;
	pthread_create(&crr_stub, NULL, &(command_receiver_and_responder_stub), NULL);
	pthread_join(crr_stub, NULL);

	pthread_join(ping_server, NULL);
	pthread_join(connection_checker, NULL);
	return 0;
}

