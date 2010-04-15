/* Keeps track of existing client
 * and clean-up dead nodes */

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

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

/* A Mutual Exclusion on the Node-Map */
sem_t nm_sem;
map <string, node> node_map;

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
		
		sem_wait(&nm_sem);
		for(int i = 0; i < dead_nodes.size(); i++)
		{
			node_map.erase(dead_nodes[i]);
		}
		sem_post(&nm_sem);

	}
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
    if ((rv = getaddrinfo(NULL, PORT2, &hints, &servinfo)) != 0) 
	{
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
	
           if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Error in listen_to_client_ping(): Failed to bind socket\n");
            continue;
        }

        break;
    }

    if (p == NULL) 
	{
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
			sem_wait(&nm_sem);
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
			sem_post(&nm_sem);
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
