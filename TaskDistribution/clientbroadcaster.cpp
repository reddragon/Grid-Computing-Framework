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

/* PORT1 is used for listening by server and broadcasting by client */
#define PORT1 "4950"    

/* PORT2 is used for sending by server and listening by client */
#define PORT2 "4951"

/* PORT 3 is used for sending commands by server and listening by client */
#define PORT3 "4952"

#define MAXBUFLEN 100

/* Time in microseconds between two client pings to the server */
#define CLIENTRESPONSETIMEGAP 200000

/* Time within which the server should respond to the client, in seconds and microseconds */
#define SERVERTIMEOUTSEC 4
#define SERVERTIMEOUTUSEC 0

/* Number of times the client retries to establish a connection with the server */
#define CONNECTIONRETRIES 100

/* Number of iterations inside the performance metric calculator */
#define PERFMETRICITERATIONS 500000

/* Time between two metric recalculations */
#define RECALCULATIONTIMEGAP 10000000

char server_address[100];
double nw_metric, pf_metric;

struct nw_metric_params
{
        int packet_size, packet_count; 
        char *server_add;
};


/*
 * List of messages sent by client
 * 
 * SERVERADDREQ		Client is requesting for the server's address
 * CLIRESP performance_metric network_metric	Client is responding with the performance and network metric
 *  
 */

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
	printf("Reliability: %d\tAverage Latency: %lf\tMean Dev: %lf\n", 
			reliability, avg_latency, mdev_latency);
	
	network_metric_value = reliability * 1.0 /((avg_latency + 2 * mdev_latency) * 1.0);
	return network_metric_value;
}


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);
   
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int resend_connection_request()
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
        fprintf(stderr, "Error: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error: Could not create socket\n");
            continue;
        }
		
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Error: Failed to bind socket\n");
        return 0;
    }
	
	/* Required to permit UDP Broadcasts */
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, 4) == -1)
	{
		fprintf(stderr, "Error: Failed to set socket options\n");
		return 0;
	}
	
	/* SERVERADDREQ message */
	strcpy(messagebuffer, "SERVERADDREQ");
	
    if ((numbytes = sendto(sockfd, messagebuffer, strlen(messagebuffer), 
		0, p->ai_addr, p->ai_addrlen)) == -1) 
	{
        perror("Error: Message could not be sent\n");
        exit(1);
    }
    freeaddrinfo(servinfo);
    printf("Successfully sent the broadcast message. Size: %d bytes, \
			Destination: %s\n", numbytes, hostname);
	close(sockfd);
	return 1;
}

void * recalculate_metrics(void *r)
{
        struct nw_metric_params *n = (struct nw_metric_params *)(r);
        
        while(1)
        {
                
                usleep(RECALCULATIONTIMEGAP);
                printf("Metrics recalculated. Performance Metric: %lf  Network Metric: %lf\n", pf_metric, nw_metric);
                pf_metric = performance_metric();
                nw_metric = network_metric(n->packet_size, n->packet_count, n->server_add);
        }
}

void start_recalculating_metrics(struct nw_metric_params n)
{
	
}

int initiate_listener()
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    
    int rv;
    int numbytes;
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
	
    if ((rv = getaddrinfo(NULL, PORT2, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "Error: %s\n", gai_strerror(rv));
        return 0;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error: Could not create socket\n");
            continue;
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
        fprintf(stderr, "Error: Failed to bind socket\n\n");
        return 0;
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
			resend_connection_request();
			continue;
		} 
	}
	if(connretriesleft <= 0)
	{
		printf("Could not connect with the server. Terminating.");
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
	strcpy(server_address, inet_ntop(their_addr.ss_family, 
			get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
	
	printf("The server address is: %s\n", server_address);
	close(sockfd);
	return 1;
}

void * command_listener(void * args)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    
    int rv;
    int numbytes;
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
	
    if ((rv = getaddrinfo(NULL, PORT3, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "Error: %s\n", gai_strerror(rv));
        return 0;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
            perror("Error: Could not create socket\n");
            continue;
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
        fprintf(stderr, "Error: Failed to bind socket\n\n");
        return 0;
    }

    freeaddrinfo(servinfo);
    printf("Command Listener Initiated...\n");	
    addr_len = sizeof their_addr;
	
   while(1)
   {
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
			resend_connection_request();
			continue;
		} 
	     }
	if(connretriesleft <= 0)
	{
		printf("No commands received yet.");
		continue;
	}
	
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) 
		continue;
	
	buf[numbytes] = '\0';
	printf("Text received: %s\n", buf);
       }
	close(sockfd);
	return NULL;
}


void initiate_responder()
{
	int sockfd;
        struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, optval = 1;
	char messagebuffer[500];
	
	printf("Calculating network metric..\n");
	nw_metric = network_metric(64, 2, server_address);
	printf("Network metric calculated.\n");
	
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
        if ((rv = getaddrinfo(server_address,PORT1, &hints, &servinfo)) != 0) 
	{
               fprintf(stderr, "Error: %s\n", gai_strerror(rv));
                return;
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
                 return;
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

int main()
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
	
	printf("Calculating performance metric.. \n");
	pf_metric = performance_metric();
	printf("Performace metric calculated.\n");

    if ((rv = getaddrinfo(hostname,PORT1, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "Error: %s\n", gai_strerror(rv));
        return 1;
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
        return 2;
    }
	
	/* Required to permit UDP Broadcasts */
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, 4) == -1)
	{
		fprintf(stderr, "Error: Failed to set socket options\n");
		return 2;
	}
	
	/* SERVERADDREQ message */
	strcpy(messagebuffer, "SERVERADDREQ");
	
    if ((numbytes = sendto(sockfd, messagebuffer, strlen(messagebuffer), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("Error: Message could not be sent\n");
        exit(1);
    }
    freeaddrinfo(servinfo);
    printf("Successfully sent the broadcast message. Size: %d bytes, Destination: %s\n", numbytes, hostname);
	close(sockfd);
	
    initiate_listener();
    pthread_t command_thread;
    pthread_create(&command_thread, NULL, &(command_listener), NULL);
    pthread_join(command_thread, NULL);
    initiate_responder();
		
    return 0;
}
