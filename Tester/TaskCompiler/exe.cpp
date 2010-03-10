#include  <unistd.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include  <sys/wait.h>
#include  <iostream>

using namespace std;

int main()
{
   int data_processed;
   int file_pipes[2];
   const char some_data[] = "123";
   pid_t fork_result;
   
   if (pipe(file_pipes) == 0) 
   {
       fork_result = fork();
       if (fork_result == (pid_t)-1) 
			 {
                 fprintf(stderr, "Fork failure");
                 exit(EXIT_FAILURE);
             }
      if (fork_result == (pid_t)0) 
	  {
		  close(2);
		  dup(file_pipes[1]);
		  close(file_pipes[1]);
		  close(file_pipes[0]);
		  
		  //execlp("g++", "g++", "-o", "t1", "t1.cpp", (char *)0);
          system("g++ t1.cpp");
		  exit(EXIT_SUCCESS);
      }
      else 
	  {
		    int st;
			waitpid(fork_result, &st, 0);
			char buff[10000];
			int br = read(file_pipes[0],buff,10000-1);
			cout << br << " bytes read" << endl;
			cout << buff << endl;
      }
  }
  exit(EXIT_SUCCESS);
}
