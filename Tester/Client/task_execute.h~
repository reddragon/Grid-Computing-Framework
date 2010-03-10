#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<string>
#include<fstream>
#include<unistd.h>
#include<cstring>
#include<sys/wait.h>
#include<sys/resource.h>
#include<cmath>
#include<sys/times.h>
using namespace std;

struct TDDFile
{
	string task_id, task_compile_command, task_execution_command;
	string task_input_file, task_output_file;
	int priority; float time_limit;
};

void wrap_up()
{
	exit(1);
}

void end_task()
{
	exit(1);
}

void die(const char *msg, int wrap_up_flag = 0)
{
	fprintf(stderr, "Error: %s\n", msg);
	if(!wrap_up_flag)end_task();
	wrap_up();
}

int compile_task(TDDFile *tf)
{
	int file_pipes[2];
	pid_t pid;
	string task_compile_command = tf->task_compile_command;
	string task_id = tf->task_id;
	if(pipe(file_pipes) == 0)
	{
		pid = fork();
		if(pid == -1)
			die("Could not fork a new process.",1);
	
		if(pid == 0)
		{
			close(2);
			dup(file_pipes[1]);
			system((char *)(task_compile_command).c_str());
			fprintf(stderr, "~");
			close(file_pipes[0]);
			close(file_pipes[1]);
			exit(0);
		}
		else
		{
			FILE *compilation_error_log;
			compilation_error_log = fopen((char*)((task_id + "_cel.log").c_str()), "w");
			int st, error_bytes;
			waitpid(pid, &st, 0);
			char error_buff[10001];
			error_bytes = read(file_pipes[0],error_buff,10000);
			error_bytes--;
			if(!error_bytes) 
			{ 
				fclose(compilation_error_log);
				return error_bytes;
			}
			fwrite(error_buff, sizeof(char), error_bytes, compilation_error_log);
			fclose(compilation_error_log);
			return error_bytes;
		}
	}
	else
	{
		fprintf(stderr, "Error: Could not create pipe while compiling task %s.\n", (char *)task_id.c_str());
		end_task();
	}
	return -1;
}

const char* sig_text[] = {
    "Success",
    "Hangup",
    "Interrupted",
    "Quit",
    "ILL",
    "TRAP",
    "Aborted",
    "7",
    "Floating Point Exception",
    "Killed",
    "10",
    "Segmentation Fault"
};


int execute_task(TDDFile *tf)
{
	int file_pipes[2];
	pid_t pid;
	pid = fork();
	if(pid == -1)
		die("Could not fork a new process.",1);
	
	if(pid == 0)
	{
		string command = tf->task_execution_command + " < " + tf->task_input_file + " > " + tf->task_output_file;
		rlimit resource_limit;
		resource_limit.rlim_cur = (int)(ceil(tf->time_limit + 0.1));
		resource_limit.rlim_max = (int)(ceil(tf->time_limit + 0.1));
		
		if(setrlimit(RLIMIT_CPU, &resource_limit) == -1)
			die("Could not set resource limits.", 1);
		printf("Executing\n");
		freopen((char *)(tf->task_input_file).c_str(), "r", stdin);
		freopen((char *)(tf->task_output_file).c_str(), "w", stdout);
		execlp("./task1", "./task1", (char *)0);
		exit(0);		
	}
	else
	{
		 int status, ret, max_real_secs = (int)ceil(tf->time_limit)*2;
        	 long ticks_per_sec = sysconf(_SC_CLK_TCK);
        	 struct tms t;
                 time_t start = time(0);
        
        	 while ((ret = waitpid(pid, &status, WNOHANG)) == 0) {
          		  if ((time(0) - start) > max_real_secs)
                		die("Killed because it seemed to have hung.");
            		  usleep(50000);
        	 }

        	if (ret < 0)
            		die("Failed to wait for child process.");

        
       		times(&t);
        	float time_taken = ((float)(t.tms_cutime + t.tms_cstime))/ticks_per_sec;
		printf("Time taken: %.2f\n", time_taken);
        	ofstream ofs;
		ofs.open((char *)((tf->task_id + "_stats.log").c_str()));
		ofs.close();
		
		FILE *execution_error_log;
		execution_error_log = fopen((char *)( (tf->task_id + "_eel.log").c_str()), "w");

		 char execution_error_message[200];
       		 if ((float)(t.tms_cutime + t.tms_cstime) > tf->time_limit*ticks_per_sec) 
		 {
			 sprintf(execution_error_message, "Exceeded time limit of %.2f seconds.\n", tf->time_limit);
            		 fprintf(stderr, "%s", execution_error_message);
			 fprintf(execution_error_log, "%s", execution_error_message);
			 fclose(execution_error_log);
			 return 1;
        	 }
        
		if (WIFEXITED(status))
            		printf("Successfully Executed\n");
        	else {
            	if (WIFSIGNALED(status)) {
                	int sig = WTERMSIG(status);
                
                if (sig < 0 || sig > 11)
		{
                    sprintf(execution_error_message, "Terminated (%d).\n", sig);
                    fprintf(stderr, "%s", execution_error_message);
		}
		else 
                    fprintf(stderr, "Terminated (%s).\n", sig_text[sig]);
            }
            else
	    {   
		sprintf(execution_error_message, "Unknown error (%d)!\n", status);
            	fprintf(stderr, "%s", execution_error_message);
	    }
	    fprintf(execution_error_log, "%s", execution_error_message);
	    fclose(execution_error_log);
	    return 1;
        }

    fclose(execution_error_log);
    }

    return 1;
}

int compile_and_execute(string tdd_file)
{
	//string tdd_file = "t1.tdd";
	TDDFile tf;
	ifstream ifs;
	ifs.open((char *)((tdd_file).c_str()));
	getline(ifs,tf.task_id);
	getline(ifs,tf.task_compile_command);
	getline(ifs,tf.task_execution_command);
	ifs >> tf.task_input_file;
	ifs >> tf.task_output_file;
	ifs >> tf.priority;
	ifs >> tf.time_limit;
	
	int compile_error_msg_len = compile_task(&tf);
	if(compile_error_msg_len == 0) printf("File compiled successfully.\n");
	else
	{
	   fprintf(stderr,"Errors/Warnings while compiling task %s. Please check the log file %s_cel.log\n", tf.task_id.c_str(), tf.task_id.c_str());
	   end_task();
	}
	execute_task(&tf);
	
	return 0;		
}
