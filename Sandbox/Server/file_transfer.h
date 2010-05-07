/* A utility to send/receive
 * files to/from the client */

#include <string>

struct fileinfo
{
	string file_name, host_name;
	int file_size;
} fi;

int get_file_size(string file_name)
{
	cout << "Finding size of: " << file_name << endl;
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


int ssockfd, sportno, snewsockfd, sn, slsize;
socklen_t  scli_len;
struct sockaddr_in sserv_addr, scli_addr;

void bind_sf_socket()
{
          
     ssockfd = socket(AF_INET, SOCK_STREAM, 0);
     if(ssockfd < 0)
     {
	     cout << "error!";
     }
     bzero((char *)&sserv_addr, sizeof(sserv_addr));
     sportno = atoi(PORT5);
     sserv_addr.sin_family = AF_INET;
     sserv_addr.sin_addr.s_addr = INADDR_ANY;
     sserv_addr.sin_port = htons(sportno);
     int optval;
     
     if(setsockopt(ssockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
     {
	   fprintf(stderr, "Failed to set socket options\n");
	   exit(1);
     }

     
     if(bind(ssockfd, (struct sockaddr *) & sserv_addr, sizeof(sserv_addr)) < 0)
	 {
		cout << "Error in binding "; exit(1);
	 }
	
      listen(ssockfd, 5);
      scli_len = sizeof(scli_addr);

}

/* Semaphores for Mutual Exclusion on the Sockets */
sem_t sf_sem;
sem_t rf_sem;

void SIGPIPE_handler(int sig)
{	
	cout << "Received the signal: " << sig << endl;
}

int send_file(void *args)
{
     fileinfo *fi = (fileinfo *)args;
     string file_name = fi->file_name, host_name = fi->host_name;
         
      snewsockfd = accept(ssockfd, (struct sockaddr *) &scli_addr, &((scli_len)));
      if(snewsockfd < 0)
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
	 cout << "Trying to send on port " << ntohs(sserv_addr.sin_port) << endl;
     struct timeval tv;
     tv.tv_sec = CLIENTTIMEOUT ;
     tv.tv_usec = 1;
     fd_set writefds;
     FD_ZERO(&writefds);
      FD_SET(snewsockfd,&writefds);
      if(select(snewsockfd+1,NULL,&writefds,NULL,&tv) == -1 )
      {
			//Time out on a socket
			cout << "Timed out on the socket" << endl;
			return 1;
      }
	 
    do
    {
      n = write(snewsockfd,buffer + mov_ptr,to_do);
      
      if (n < 0) 
      {	        
		 //TODO
		 //Have to handle this error
	     return 2;
      }
      to_do -= n;
      cout << "Written " << n << " bytes " << to_do << " bytes left for " <<  file_name <<  endl;

      mov_ptr += n;
      if(to_do == 0) break;

     } while(1);
	 
      free(buffer);
      return 0;
}

int receive_file(fileinfo args)
{  
	string file_name = args.file_name, host_name = args.host_name;
    int file_size = args.file_size;
  
    int sockfd, portno;
    long n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
  
    char *buffer = new char[MAXSIZE];
    if(buffer == 0)
    {
	    cout << "Failed to allocated memory" << endl;
	    exit(1);
    }
    memset(buffer, 0, sizeof(buffer));
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
	
    int wait_sec = 50;
    while (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        if(wait_sec == 0)
	{
		printf("Error in connecting\n");
		return 1;
	}
	else { wait_sec--; usleep(100000); }
    }
	
   
   int to_receive = file_size;
   while(to_receive > 0)
   {
		n = read(sockfd,buffer,sizeof(buffer));
		if(n <= 0) break;

		to_receive -= n;
		fwrite(buffer,1,n,fp);
   }
   fclose(fp); 
   delete [] buffer;
   return 0;
}


