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

#define PORT1 "6366" 
#define PORT2 "6367"
#define PORT3 "6368"
#define PORT4 "6369"
#define PORT5 "6370"
#define PORT6 "6371"
#define PORT7 "6372"
#define CONNECTIONRETRIES 100
#define RETRYGAP 200000

char server_address[100];

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
	strcpy(server_address, inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
		
	printf("Message from server: %s\n", buf);	
	printf("The server address is: %s\n", server_address);
	close(sockfd);
	return 1;

}

int main()
{
	get_server_address();
}

