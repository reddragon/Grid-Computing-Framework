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

#define PORT1 "4950" 
#define PORT2 "4951"

#define MAXBUFLEN 100
#define MAXHOSTS 200
#define HOSTCONNTIMEOUT 5
#define CLEANERSLEEPTIME 500

struct host
{
	char host_name[100];
	int last_seen;
	double nw_metric, pf_metric;
} host_list[MAXHOSTS];

struct response_to_host
{
	char host_name[100];
	char message[100];
};

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void * cleanup_dead_hosts(void * arg)
{
	
	while(1)
	{
		int i, cur_time = time(NULL);
		for(i = 0; i < MAXHOSTS; i++)
		{
			if(host_list[i].last_seen != -1 && (cur_time - host_list[i].last_seen) >= HOSTCONNTIMEOUT)
			{
				host_list[i].last_seen = -1;
				printf("Connection with %s has timed out\n", host_list[i].host_name);
				strcpy(host_list[i].host_name, "");
			}
		}
		usleep(CLEANERSLEEPTIME);
	}
}


int insert_new_host(char *host_name)
{
	int i;
	int cur_time = time(NULL);
	short placed = 0;
	for(i = 0; i < MAXHOSTS && !placed; i++)
	{
		if(!strcmp(host_list[i].host_name, host_name))
		{
				host_list[i].last_seen = cur_time;
				placed = i + 1;
				break;
		}
		
		if(!placed && host_list[i].last_seen == -1 && !strcmp(host_list[i].host_name, ""))
		{
			strcpy(host_list[i].host_name, host_name);
			printf("Host %s now placed at location %d\n", host_list[i].host_name, i);
			host_list[i].last_seen = cur_time;
			placed = i + 1;
			break;
		}
	}
	
	return placed;
}

void start_auto_cleanup()
{
	void *arg;
	pthread_t thread;
    pthread_create (&thread, NULL, &(cleanup_dead_hosts), arg);
}

void * send_reponse(void *response)
{
	struct response_to_host *r = (struct response_to_host *) response;
	printf("Trying to send to %s, the message %s\n", r->host_name, r->message);
	
	int sockfd;
    struct addrinfo hints, *servinfo, *p;
	int rv, numbytes;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(r->host_name,PORT2, &hints, &servinfo)) != 0) 
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

    if (p == NULL) {
        fprintf(stderr, "Error: Failed to bind socket\n");
        return NULL;
    }
	
    if ((numbytes = sendto(sockfd, r->message, strlen(r->message), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("Error: Message could not be sent\n");
        exit(1);
    }
    freeaddrinfo(servinfo);
    //printf("Successfully sent the resonse message: %s, to destination: %s\n", r->message, r->host_name);
	close(sockfd);
	return NULL;
}

void respond_to_client(struct response_to_host response)
{
	pthread_t thread;
	pthread_create (&thread, NULL, (&send_reponse), &response);
	pthread_join(thread,NULL);
}

void * print_active_hosts(void * arg)
{
	int i;
	while(1)
	{
		usleep(3000000);
		for(i = 0; i < MAXHOSTS; i++)
		{
			if(host_list[i].last_seen != -1)
			{
				printf("Host: %s  Performance Metric: %lf\tNetwork Metric: %lf\n", 
				host_list[i].host_name, host_list[i].pf_metric, host_list[i].nw_metric);
			}
		}
	}
	printf("\n\n");
	
}

void show_hosts()
{
	pthread_t thread;
	void *arg;
	pthread_create(&thread, NULL, (&print_active_hosts), arg);
}

int * result;

void * start_server(void * args)
{
    int sockfd, i;
    struct addrinfo hints, *servinfo, *p;
    
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
	for(i = 0; i < MAXHOSTS; i++)
		host_list[i].last_seen = -1;
	
	show_hosts();

    if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo)) != 0) {
        fprintf(stderr, "Server Broadcast Listener error: %s\n", gai_strerror(rv));
        *result = 1; return result; 
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
            perror("Server Broadcast Listener error: Could not create socket\n");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server Broadcast Listener error: Failed to bind socket\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Server Broadcast Listener error: Failed to bind socket\n\n");
        *result = 2; return result;
    }

    freeaddrinfo(servinfo);
	start_auto_cleanup();
    while(1)
	{
		//printf("Waiting for a message...\n");

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) 
	   {
			perror("recvfrom");
			exit(1);
       }
	   buf[numbytes] = '\0';

		strcpy(contacting_client, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		
		int result = insert_new_host(contacting_client);
		
		if(!result)
		{
			printf("Server cannot accept any more client requests!");
		}
		else
		{
			struct response_to_host response;
			strcpy(response.host_name, contacting_client);
			if(!strcmp(buf, "SERVERADDREQ"))
			{
				strcpy(response.message, "SERVER");
				respond_to_client(response);
			}
			else
			{
				double nw_metric, pf_metric;
				char command[100];
				sscanf(buf, "%s %lf %lf", command, &nw_metric, &pf_metric);
				host_list[result-1].nw_metric = nw_metric;
				host_list[result-1].pf_metric = pf_metric;
			}
			
		}
	
    }  
    close(sockfd);

    *result = 0; return result;
}
