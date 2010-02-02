#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_SIZE 100000

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
	long n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	FILE *fp;
	fp=fopen("f.mp3","wb");
    char buffer[MAX_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
/*    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");*/
    while(1)
	{
		bzero(buffer,MAX_SIZE);
		n = read(sockfd,buffer,sizeof(buffer));
    
		if (n <= 0) 
			break;
		printf("Size of File is %ld\n",n);
	
		int i=0;	 
		fwrite(buffer,1,n,fp);
	}
	fclose(fp); 
	/* while(n!=0)
	 {
		if(buffer[i] == '\z')
			break;
		fprintf(fp,"%c",buffer[i++]);
		n--;
	 }    */
	
	

    return 0;
}
