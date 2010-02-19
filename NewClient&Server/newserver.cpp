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

#define PORT1 "6366" 
#define PORT2 "6367"
#define PORT3 "6368"
#define PORT4 "6369"
#define PORT5 "6370"
#define PORT6 "6371"
#define PORT7 "6372"

#define SERVERPINGGAP 400000

struct node
{
	char node_address[100];

};


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
    printf("Pinged\n");
    usleep(SERVERPINGGAP);
   }

}

int main()
{
	pthread_t ping_thread;
	pthread_create(&ping_thread, NULL, &(broadcast_ping), NULL);
	pthread_join(ping_thread, NULL);
	return 0;
}
