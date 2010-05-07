/* A utility for Sending and receiving
 * commands from the server and handling them. 
 * 
 * Also pings the server and receives pings
 * from the server.
 * */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct command
{
	string command_str, client;
};

char server_address[100];


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);
   
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


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


fileinfo sf;
void * receive_commands(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char tbuf[MAXBUFLEN];
    
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
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
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
	
			if(cmd == "COLLECTFILE")
			{
				cm >> arg0 >> arg1;
				stringstream ss(arg1);
				int file_size;
				ss >> file_size;
				command c;
				c.client = (string)server_address;
				c.command_str = "COLLECTFILEACK " + arg0;
				send_command_to_server(c);
				fi.file_name = arg0; fi.host_name = c.client; fi.file_size = file_size;
				if(!receive_file(&fi))
				{
					cout << fi.file_name << " received successfully" << endl;
					
					string task = arg0.substr(0,arg0.size()-((string)".tar.gz").size());
					Task tf;
					int ce_ret = task_ce(task, tf);
					if(ce_ret == 0)
						cout << "Executed properly" << endl;
					
					else
					{
						cout << "Errors in compilation / execution" << endl;
						continue;
					}
					string result_archive = task + "_op.tar.gz";
					int ra_fs = get_file_size(result_archive);
					stringstream ss;
					ss << "COLLECTRES" << " " << result_archive << " " << ra_fs;
					
					command collect_res;
					cout << "Sending: " << ss.str() << endl;
					collect_res.command_str = ss.str();
					collect_res.client = (string)server_address;
					send_command_to_server(collect_res);	
					
					sf.file_name = result_archive;
					sf.file_size = ra_fs;
					sf.host_name = server_address; 
					
					//TODO
					/* File size may be more than 2x10^9 bytes? */
					int sf_err = send_file(&sf);
					if(sf_err)
					{
						cout << "Error with send_file." << endl;
						continue;
					}

					cout << "Done with execution, cleaning up." << endl;
					task_file_cleanup(tf);
					cout << "Task files cleaned" << endl;


				}
				else
				{
					cout << "Could not receive file" << endl;
					continue;
				}
			}
		}
  
     }
  	delete [] buf;
	freeaddrinfo(servinfo);
	close(sockfd);
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
			 continue; 
	   }
			
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
