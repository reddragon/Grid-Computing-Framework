#include <cstdlib>
#include <cstdio>
#include "interface.h"

int send_ports[20];
int recv_ports[20];

struct fileinfo
{
	string file_name, host_name;
	int file_size, port_id;

};

fileinfo fi;

void send_to_command_view3(string str)
{	
	set_command_view((char *)str.c_str());		
}
char to_send1[100];

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

int ssockfd[20], sportno, snewsockfd[20];
socklen_t  scli_len;
struct sockaddr_in sserv_addr, scli_addr[20];

void bind_sf_socket(int port_id)
{   
     ssockfd[port_id] = socket(AF_INET, SOCK_STREAM, 0);
     if(ssockfd < 0)
     {
	     cout << "Error in socket creation.";
     }
     bzero((char *)&sserv_addr, sizeof(sserv_addr));
     sportno = send_ports[port_id];
     sserv_addr.sin_family = AF_INET;
     sserv_addr.sin_addr.s_addr = INADDR_ANY;
     sserv_addr.sin_port = htons(sportno);
     int optval;
     
      if(setsockopt(ssockfd[port_id], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
     {
	   fprintf(stderr, "Failed to set socket options\n");
	   exit(1);
     }

     
     if(bind(ssockfd[port_id], (struct sockaddr *) & sserv_addr, sizeof(sserv_addr)) < 0)
     {
		cout << "Error in binding "; exit(1);
     }
	
     listen(ssockfd[port_id], 5);
}



int send_file(void *args)
{
     
      fileinfo *fi = (fileinfo *)args;
      string file_name = fi->file_name, host_name = fi->host_name;
      int port_id = fi->port_id;      

      snewsockfd[port_id] = accept(ssockfd[port_id], (struct sockaddr *) &scli_addr[port_id], &((scli_len)));
      if(snewsockfd[port_id] < 0)
      {
	      cout << "Error in accepting. Here."; exit(1);
      }
	  	
     FILE *fp;
     fp=fopen(file_name.c_str(),"rb");
     int lsize, n;
     fseek(fp,0,SEEK_END);
     lsize=ftell(fp);
     rewind(fp);
     char *buffer;	 
     buffer=(char *)malloc(sizeof(char)*lsize); 
     if(buffer == NULL)
     {
	     cout << "Problem in allocating memory";
	     exit(1);
     }
     memset(buffer, 0, sizeof(buffer));

     fread(buffer,1,lsize,fp);
     fclose(fp);
     fflush(stdout);
     
     int to_do = lsize;
     int mov_ptr = 0;
	 
	 
     do
    {
         struct timeval tv;
	 tv.tv_sec = 10;
	 tv.tv_usec = SUPERVISORTIMEOUTUSEC;
	 fd_set writefds;
	 FD_ZERO(&writefds);
	 FD_SET(snewsockfd[port_id],&writefds);
	 
	if(!(select(snewsockfd[port_id]+1,NULL,&writefds,NULL,&tv) > 0 && FD_ISSET(snewsockfd[port_id], &writefds)))
     	{
			//Time out on a socket
			cout << "Timed out on the socket" << endl;
			free(buffer);
			return 1;
     	}
	 
      n = send(snewsockfd[port_id],buffer + mov_ptr,to_do, 0);
      if (n < 0) 
      {	     
	     cout << "Supervisor has crashed" << endl;
	     send_to_command_view3("Supervisor has crashed");	
	     return 2;
      }
      to_do -= n;
      
      mov_ptr += n;
      if(to_do == 0) break;

     } while(1);
	 
      free(buffer);
      return 0;
}

int receive_file(void * args)
{
    fileinfo * f = (fileinfo *) args;
    string file_name = f->file_name, host_name = f->host_name;
    int port_id = f->port_id;

    int sockfd, portno;
    long n;
    struct sockaddr_in serv_addr;
    struct hostent *supervisor;
		
    char *buffer = (char *)malloc(sizeof(char) * (f->file_size));
    if(buffer == NULL)
    {
	cout << "Error in allocation";
	return 1;
    }
    memset(buffer, 0, sizeof(buffer));

    FILE *fp;
    fp=fopen(file_name.c_str(),"wb");
	
    portno = recv_ports[port_id];
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        printf("Error in opening socket");
	exit(1);
    }
    supervisor = gethostbyname(host_name.c_str());
    if (supervisor == NULL) 
    {
        fprintf(stderr,"Error, no such host\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)supervisor->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         supervisor->h_length);
    serv_addr.sin_port = htons(portno);

    cout << "Trying to receive on port " << ntohs(serv_addr.sin_port) << endl;
    sprintf(to_send1,"Trying to receive on port %d",ntohs(serv_addr.sin_port));
    send_to_command_view3(to_send1);

    int wait_sec = 1000;
    
     while (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        if(wait_sec == 0)
	{
			printf("Error in connecting");
			return 1;
	}
	else { wait_sec--; usleep(1000); }
    }
   
 
   int to_receive = f->file_size;
    	 
	 
   while(to_receive > 0)
   {
	 struct timeval tv;
	 tv.tv_sec = SUPERVISORTIMEOUTSEC;
	 tv.tv_usec = SUPERVISORTIMEOUTUSEC;
	 fd_set readfds;
	 FD_ZERO(&readfds);
	 FD_SET(sockfd,&readfds);
	 
	 if(!(select(sockfd+1,&readfds, NULL,NULL,&tv) > 0 && FD_ISSET(sockfd, &readfds)))
       	 {
			cout << "Timed out on the socket" << endl;
			free(buffer);
			return 1;
       	 }	
	 	
		n = recv(sockfd,buffer,sizeof(buffer),0);

		if(n <= 0) break;

		to_receive -= n;
		fwrite(buffer,1,n,fp);
   }
   fclose(fp); 
   free(buffer);
   return 0;
}
