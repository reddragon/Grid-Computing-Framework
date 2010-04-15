#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_SIZE 10000

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
	 socklen_t clilen;
     char *buffer;
     struct sockaddr_in serv_addr, cli_addr;
	 long lsize;
	 printf("Here");
	 fflush(stdout);
	 FILE *fp;
	 fp=fopen(argv[2],"rb");
	 if (fp==NULL) {printf("File error"); exit (1);}
	 
	 
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     
	 
	 
	/* char temp[2];
	temp[1]='\0';
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);*/
/*	 while(!feof(fp))
	 {
		fscanf(fp,"%c",&temp[0]);
		strcat(buffer,temp);
	 }
	 strcat(buffer,"");*/
	 fseek(fp,0,SEEK_END);
	 lsize=ftell(fp);
	 rewind(fp);
	 
	 buffer=(char *)malloc(sizeof(char)*lsize); 
	 fread(buffer,1,lsize,fp);
	 fclose(fp);
	 fflush(stdout);
     n = write(newsockfd,buffer,lsize);
	 printf("Bytes sent: %d\n\n",n);
     if (n < 0) error("ERROR writing to socket");
//	 fclose(fp);
     return 0; 
}
