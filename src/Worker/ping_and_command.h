/* A utility for Sending and receiving
 * commands from the supervisor and handling them. 
 * 
 * Also pings the supervisor and receives pings
 * from the supervisor.
 * */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <statgrab.h>


struct command
{
	string command_str, client;
};

char supervisor_address[100];
float nice_val=0,cpu_use=0;
char rec_window[200];
Task curr;




void cpu_usage()
{
	sg_cpu_percents *cpu_percent;
	sg_init();
	sg_snapshot();
	cpu_percent = sg_get_cpu_percents();

	nice_val=cpu_percent->nice;
	cpu_use=100.0-cpu_percent->idle;
}

void send_to_command_view(string str)
{	
	set_command_view((char *)str.c_str());		
}


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);
   
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void send_command_to_supervisor(command arg)
{
	int sockfd;
        struct addrinfo hints, *servinfo, *p;
	int rv, numbytes;
	char messagebuffer[500];
	
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(arg.client.c_str(),PORT4, &hints, &servinfo)) != 0) 
		{
               fprintf(stderr, "Error in send_command_to_supervisor(): %s\n", gai_strerror(rv));
               exit(1);
        }

        for(p = servinfo; p != NULL; p = p->ai_next) 
		{
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
				{
                    perror("Error in send_command_to_supervisor(): Could not create socket\n");
                     continue;
                 }
		break;
       }
	
        if (p == NULL) 
		{
                 fprintf(stderr, "Error in send_command_to_supervisor(): Failed to bind socket\n");
                 exit(1);
        }	
	
	strcpy(messagebuffer, arg.command_str.c_str());	
	
	if ((numbytes = sendto(sockfd, messagebuffer, 
			strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) 
	{
			perror("Error in send_command_to_supervisor(): Message could not be sent\n");
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


fileinfo sf;
void * receive_commands(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    
    int rv, numbytes;
    struct sockaddr_storage their_addr;
	
    char *buf = new char[MAXBUFLEN];
    if(buf == 0)
    {
	    cout << "Error allocating memory";
	    exit(1);
    }
    size_t addr_len;
    char s[INET6_ADDRSTRLEN], contacting_mc[100];
	

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
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
	  	 {
			perror("recvfrom");
			exit(1);
       		}
	   	buf[numbytes] = '\0';

		strcpy(contacting_mc, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		if(strcmp(contacting_mc, supervisor_address) == 0)
		{
			cout << "Received the command: " << buf << " from the supervisor" << endl;
			stringstream cm((string)buf);
			sprintf(rec_window,"\nReceived the command: %s from the supervisor", buf);
			send_to_command_view(rec_window);
			string cmd, arg0, arg1, arg2;
			cm >> cmd;	
	
			if(cmd == "COLLECTFILE")
			{			
				cm >> arg0 >> arg1 >> arg2;
				stringstream ss(arg1 + " " + arg2);
				int file_size, port_id;
				ss >> file_size >> port_id;
				cout << "Waiting for " << arg0 << endl;		
				command c;
				c.client = (string)supervisor_address;
				c.command_str = "COLLECTFILEACK " + arg0;
				send_command_to_supervisor(c);
				fi.file_name = arg0; fi.host_name = c.client; fi.file_size = file_size;
				fi.port_id = port_id;

				if(!receive_file(&fi))
				{
					cout << fi.file_name << " received successfully" << endl;
					
					string task = arg0.substr(0,arg0.size()-((string)".tar.gz").size());
					
					sprintf(rec_window,"%s received successfully", fi.file_name.c_str());
					send_to_command_view((string)rec_window);
					
					char string_to_exec[200];
					cpu_usage();
					read_tdd(curr,task);
					sprintf(string_to_exec,"Task ID: %s\nTask Priority: %d\nTask Limit: %.2lf\nCPU Nice: %.2f\nCPU Busy: %.2f\n",task.c_str(),curr.priority,curr.time_limit,nice_val,cpu_use);
					set_execution_view(string_to_exec);
					
					Task tf;
					int ce_ret = task_ce(task, tf);
					if(ce_ret == 0)
					{	cout << "Compilation and Execution Successful." << endl;
						send_to_command_view("Compilation and Execution Successful.");
					}
					
					else
					{
						cout << "Errors in Compilation / Execution" << endl;
						send_to_command_view("Errors in compilation / execution");
						continue;
					}
					string result_archive = task + "_op.tar.gz";
					int ra_fs = get_file_size(result_archive);
					stringstream ss;
					ss << "COLLECTRES" << " " << result_archive << " " << ra_fs;
					
					command collect_res;
					cout << "Sending: " << ss.str() << endl;
					collect_res.command_str = ss.str();
					collect_res.client = (string)supervisor_address;
					send_command_to_supervisor(collect_res);	
					
					sf.file_name = result_archive;
					sf.file_size = ra_fs;
					sf.host_name = supervisor_address; 
					sf.port_id = port_id;
					
					strcpy(rec_window,"Sending: ");
					strcat(rec_window,ss.str().c_str());
					send_to_command_view((string)rec_window);
					
					//TODO
					/* File size may be more than 2x10^9 bytes? */
					int sf_err = send_file(&sf);
					if(sf_err)
					{
						cout << "Error with send_file." << endl;
						send_to_command_view("Error with send_file.");
						continue;
					}

					cout << "Done with execution, cleaning up." << endl;
					send_to_command_view("Done with execution, cleaning up." );
					task_file_cleanup(tf);
					cout << "Task files cleaned" << endl << endl;					
					send_to_command_view("Task files cleaned");

				}
				else
				{
					cout << "Could not receive file" << endl;
					send_to_command_view("Could not receive file");
					continue;
				}
			}
		}
  
     }
  	delete [] buf;
	freeaddrinfo(servinfo);
	close(sockfd);
}


int get_supervisor_address()
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
        fprintf(stderr, "Error in get_supervisor_address(): %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error in get_supervisor_address(): Could not create socket\n");
            send_to_command_view("Error in get_supervisor_address()");
            continue;
        }
	
      int optval;
      if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
      {
	   fprintf(stderr, "Failed to set socket options\n");
	   send_to_command_view("Failed to set socket options\n");
	   exit(1);
      }
      
      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
      {
            close(sockfd);
            perror("Error: Failed to bind socket\n");
            send_to_command_view("Error: Failed to bind socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "Error in get_supervisor_address(): Failed to bind socket\n\n");
        send_to_command_view("Error in get_supervisor_address(): Failed to bind socket\n\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    printf("Waiting for a message...\n");
	send_to_command_view("Waiting for a message...\n");
	addr_len = sizeof their_addr;
	
	while((connretriesleft--) > 0)
	{
		timeout.tv_sec = SUPERVISORTIMEOUTSEC;
		timeout.tv_usec = SUPERVISORTIMEOUTUSEC;
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
			send_to_command_view("The connection timed out. Attempting again.\n");
			usleep(RETRYGAP);
			continue;
		} 
	}
	if(connretriesleft <= 0)
	{
		printf("Could not connect with the supervisor. Terminating.\n");
		send_to_command_view("Could not connect with the supervisor. Terminating.\n");
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
	strcpy(supervisor_address, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		
	printf("Message from supervisor: %s\n", buf);	
	printf("The supervisor address is: %s\n\n", supervisor_address);
	close(sockfd);
	
	strcpy(rec_window,"Message from supervisor: ");
	strcat(rec_window,buf);
	send_to_command_view(rec_window);
	
	strcpy(rec_window,"The supervisor address is: ");
	strcat(rec_window,supervisor_address);
	send_to_command_view((string)rec_window);
	
	return 1;

}

void * check_connection_with_supervisor(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char tbuf[MAXBUFLEN];
    
    int rv, numbytes;
    struct sockaddr_storage their_addr;
    size_t addr_len;
    struct timeval timeout;
    int connretriesleft = CONNECTIONRETRIES;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
    if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Error in check_connection_with_supervisor(): %s\n", gai_strerror(rv));
        send_to_command_view("Error in check_connection_with_supervisor()");
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error in check_connection_with_supervisor(): Could not create socket\n");
            send_to_command_view("Error in check_connection_with_supervisor(): Could not create socket\n");
            continue;
        }
	
     int optval;
     if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
     {
	   fprintf(stderr, "Failed to set socket options\n");
	   send_to_command_view("Failed to set socket options\n");
	   exit(1);
     }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
            close(sockfd);
            perror("Error in check_connection_with_supervisor(): Failed to bind socket\n");
            send_to_command_view("Error in check_connection_with_supervisor(): Failed to bind socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) 
    {
        fprintf(stderr, "Error in check_connection_with_supervisor(): Failed to bind socket\n\n");
        send_to_command_view("Error in check_connection_with_supervisor(): Failed to bind socket\n\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    addr_len = sizeof their_addr;
   
	while((connretriesleft--) > 0)
	{
		timeout.tv_sec = SUPERVISORTIMEOUTSEC;
		timeout.tv_usec = SUPERVISORTIMEOUTUSEC;
		fd_set masterfds, readfds;
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
			 continue; 
	   }
			
		else
		{
			printf("The connection timed out. Attempting again.\n");
			send_to_command_view("The connection timed out. Attempting again.\n");
			usleep(RETRYGAP);
			continue;
		} 
	}
	if(connretriesleft <= 0)
	{
		printf("Could not connect with the supervisor. Terminating.\n");
		send_to_command_view("Could not connect with the supervisor. Terminating.\n");
		close(sockfd);
		exit(1);
	}
}

void * ping_the_supervisor(void * args)
{
	int sockfd;
        struct addrinfo hints, *servinfo, *p;
	int rv, numbytes;
	char messagebuffer[500];
	
	printf("Calculating network metric..\n");
	nw_metric = network_metric(64, 2, supervisor_address);
	printf("Network metric calculated.\n");
	printf("Calculating performance metric..\n");
	pf_metric = performance_metric();
	printf("Performance metric calculated.\n");
        printf("Metrics calculated. Performance Metric: %lf  Network Metric: %lf\n", pf_metric, nw_metric);

	sprintf(rec_window,"Metrics calculated. \nPerformance Metric: %lf  Network Metric: %lf", pf_metric, nw_metric);
	send_to_command_view((string)rec_window);
	
	struct nw_metric_params nmp;
	nmp.packet_size = 64;
	nmp.packet_count = 2;
	nmp.supervisor_add = (char *) malloc(sizeof(char) * 100);
	strcpy(nmp.supervisor_add , supervisor_address);
	
	
	pthread_t thread;
	pthread_create (&thread, NULL, &(recalculate_metrics), (&nmp));
	memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

		printf("supervisor address %s \n\n", supervisor_address);
        if ((rv = getaddrinfo(supervisor_address,PORT2, &hints, &servinfo)) != 0) 
		{
               fprintf(stderr, "Error: %s\n", gai_strerror(rv));
                return NULL;
        }

        for(p = servinfo; p != NULL; p = p->ai_next) 
		{
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
				{
                    perror("Error: Could not create socket\n");
                    send_to_command_view("Error: Could not create socket\n");
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
		usleep(WORKERRESPONSETIMEGAP);
		sprintf(messagebuffer, "WORKERRESP %lf %lf", nw_metric, pf_metric);
		if ((numbytes = sendto(sockfd, messagebuffer, 
			strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) 
		{
			perror("Error: Message could not be sent\n");
			send_to_command_view("Error: Message could not be sent\n");
			exit(1);
		}
	}
	freeaddrinfo(servinfo);
	close(sockfd);
}
