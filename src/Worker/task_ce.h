#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <sys/resource.h>
#include <cmath>
#include <sys/times.h>
#include <sys/time.h>

#include "interface.h"

using namespace std;

struct Task
{
	string task_id, task_execution_command;
	vector<string> task_input_file, task_output_file;
	vector<string> task_compile_command, task_source_file;
	int priority; float time_limit;
};

void send_to_command_view2(string str)
{	
	set_command_view((char *)str.c_str());		
}

char to_send[100];

int wrap_up()
{
	return 2;
}

int end_task()
{
	return 1;
}

int die(const char *msg, int wrap_up_flag = 0)
{
	fprintf(stderr, "Error: %s\n", msg);
	if(!wrap_up_flag)end_task();
	return wrap_up();
}

int compile_task(Task *tf)
{
	int file_pipes[2];
	pid_t pid;
	string task_id = tf->task_id;

        cout << "Compiling Using: " << endl;
	for(int j = 0; j < tf->task_compile_command.size(); j++)	
		cout << tf->task_compile_command[0] << endl;
	
	if(pipe(file_pipes) == 0)
	{
		pid = fork();
		if(pid == -1)
			return die("Could not fork a new process.",1);
	
		if(pid == 0)
		{
			close(2);
			dup(file_pipes[1]);
			for(int j = 0; j < tf->task_compile_command.size(); j++)
			{
				system((char *)(tf->task_compile_command[j]).c_str());
			}
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
				cout << "Compiled Successfully" << endl;
				send_to_command_view2("Compiled Successfully");
				return error_bytes;
			}
			fwrite(error_buff, sizeof(char), error_bytes, compilation_error_log);
			fclose(compilation_error_log);
			char msg[50];
			sprintf(msg, "Error in compiling task %s.",(char *)task_id.c_str());
			send_to_command_view2(msg);
			return die(msg);
		}
	}
	else
	{
		char msg[100];
		sprintf(msg, "Error: Could not create pipe while compiling task %s.\n", (char *)task_id.c_str());
		send_to_command_view2(msg);
		return die(msg,1);
	}
	return 0;
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


int execute_task(Task *tf)
{
	pid_t pid;
	pid = fork();
	if(pid == -1)
		{ die("Could not fork a new process.",1);
	 	  send_to_command_view2("Could not fork a new process.");
	 	}
	int status, ret, max_real_secs = (int)ceil(tf->time_limit)*2;
        long ticks_per_sec = sysconf(_SC_CLK_TCK);
        struct tms t;
	float time_taken_2 = ((float)(t.tms_cutime + t.tms_cstime))/ticks_per_sec;
	if(time_taken_2 < 0) time_taken_2 = 0;
	timeval t1, t2;
	gettimeofday(&t1, NULL);
	
	if(pid == 0)
	{
		string command = tf->task_execution_command;
		cout << "Execution Command: " << command << endl;
		rlimit resource_limit;
		resource_limit.rlim_cur = (int)(ceil(tf->time_limit + 0.1));
		resource_limit.rlim_max = (int)(ceil(tf->time_limit + 0.1));
		
		if(setrlimit(RLIMIT_CPU, &resource_limit) == -1)
			return die("Could not set resource limits.", 1);
		cout << "Executing" << endl;
		send_to_command_view2("Executing");
		system( (char *)command.c_str());
		exit(0);		
	}
	else
	{
		
                 time_t start = time(NULL);
        
        	 while ((ret = waitpid(pid, &status, WNOHANG)) == 0) {
          		  if ((time(NULL) - start) > max_real_secs)
                		return die("Killed because it seemed to have hung.");
            		  usleep(50000);
        	 }
                
                gettimeofday(&t2, NULL);
                float time_taken = (t2.tv_sec + (t2.tv_usec * 1.0)/(1000000)) - (t1.tv_sec + (t1.tv_usec * 1.0)/(1000000));
                
                
        	if (ret < 0)
            		return die("Failed to wait for child process.",1);

        	
		printf("Time taken: %.2f\n", time_taken);
		sprintf(to_send,"Time taken: %.2f", time_taken);
		send_to_command_view2(to_send);
		
        	ofstream ofs;
		ofs.open((char *)((tf->task_id + "_stats.log").c_str()));
		ofs << time_taken;
		ofs.close();
		
		FILE *execution_error_log;
		execution_error_log = fopen((char *)( (tf->task_id + "_eel.log").c_str()), "w");

		 char execution_error_message[200];
       		 if (time_taken > tf->time_limit) 
		 {
			 sprintf(execution_error_message, "Exceeded time limit of %.2f seconds.\n", tf->time_limit);
         		 fprintf(stderr, "%s", execution_error_message);
			 fprintf(execution_error_log, "%s", execution_error_message);
			 fclose(execution_error_log);
			 return 1;
 	        }
        
		if (WIFEXITED(status))
            	{	printf("Successfully Executed\n");
			send_to_command_view2("Successfully Executed");
		}
        	else {
            	if (WIFSIGNALED(status)) {
                	int sig = WTERMSIG(status);
                
                if (sig < 0 || sig > 11)
		{
                    sprintf(execution_error_message, "Terminated (%d).\n", sig);
                    send_to_command_view2(execution_error_message);
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

    return 0;
}

int read_tdd(Task &tf, string task_id)
{
	
	string tdd_file = task_id + ".tdd";
	
	ifstream ifs;
	ifs.open((char *)((tdd_file).c_str()));
	int chars = 0; char tc;
	while(ifs >> tc) chars++;
	if(chars == 0) return 1; 	
	ifs.close();

	ifs.open((char *)((tdd_file).c_str()));
	getline(ifs,tf.task_id);
	int tc_count, j;
	ifs >> tc_count;
	for(j = 0; j < tc_count; j++)
	{
		string temp;
		while(getline(ifs,temp) && temp == "");
		tf.task_compile_command.push_back((string)temp);
	}
	
	getline(ifs,tf.task_execution_command);
	
	int ti_count;
	ifs >> ti_count;
	for(j = 0; j < ti_count; j++)
	{
		string temp;
		ifs >> temp;
		tf.task_input_file.push_back(temp);
	}
	
	int to_count;
	ifs >> to_count;
	for(j = 0; j < to_count; j++)
	{
		string temp;
		ifs >> temp;
		//cout << temp << endl;
		tf.task_output_file.push_back(temp);
	}
	ifs >> tf.priority;
	ifs >> tf.time_limit;
	
	int ts_count;
	ifs >> ts_count;
	for(j = 0; j < ts_count; j++)
	{
		string temp;
		ifs >> temp;
		tf.task_source_file.push_back(temp);
	}
	ifs.close();
	return 0;
}

void task_file_cleanup(Task tf)
{
	int i;
	string command;

	for(i = 0; i < tf.task_input_file.size(); i++)
	{
		command = "rm " + tf.task_input_file[i];
		system(command.c_str());
	}

	for(i = 0; i < tf.task_output_file.size(); i++)
	{
		command = "rm " + tf.task_output_file[i];
		system(command.c_str());
	}

	for(i = 0; i < tf.task_source_file.size(); i++)
	{
		command = "rm " + tf.task_source_file[i];
		system(command.c_str());
	}
	
	command = "rm " + tf.task_id;
	system((command + (string)".tar").c_str());
	//system((command + (string)".tar.gz").c_str());
	//system((command + (string)"_op.tar").c_str());
	system((command + (string)"_op.tar.gz").c_str());
	system((command + (string)"_cel.log").c_str());
	system((command + (string)"_eel.log").c_str());
	system((command + (string)"_zel.log").c_str());
	system((command + (string)"_stats.log").c_str());
	system((command + (string)".tdd").c_str());
	
}

int task_ce(string tid, Task &tf)
{
	string command = "gunzip -f " + tid + ".tar.gz 2> " + tid + "_zel.log"; 
	system(command.c_str());
	command = "tar xf " + tid + ".tar 2>> " + tid + "_zel.log";
	system(command.c_str());

        ifstream zefs;
	zefs.open((char *)((tid + "_zel.log").c_str())); 
	bool unzip_error = 0;
	char temp;
	while(zefs >> temp) { unzip_error = 1; break; }
	zefs.close();	
	if(unzip_error) 
        { 
		cout << "File received is possibly corrupted. Aborting execution." << endl;
		send_to_command_view2("\nFile received is possibly corrupted. Aborting execution.");		
		return 1; 
        }


	ofstream ofs;
	ofs.open((char *)((tid + "_cel.log").c_str()));
	ofs.close();
	ofs.open((char *)((tid + "_eel.log").c_str()));
	ofs.close();
	ofs.open((char *)((tid + "_stats.log").c_str()));
	ofs.close();
	
	//Task tf;
	int tret = read_tdd(tf, tid);
	if(tret == 1)
	{
		cout << "File received is probably corrupt. Aborting..";
		send_to_command_view2("File received is probably corrupt. Aborting..");
		return 1;
	}	
	int cret, eret;
	cret = compile_task(&tf);
	if(cret)
	{
		if(cret == 1)
		{
			//TODO
			//Handle this
			
		}
		else if(cret == 2)
		{
			//TODO
			//Handle this
			
		}
	}
	
	eret = 0;
	if(cret == 0)
	{
		eret = execute_task(&tf);
		if(eret == 2)
		{
			//TODO
			//Handle this
		}
	}
	string cur_task_id = tid;
	command = "tar cf " + cur_task_id + "_op.tar " + cur_task_id + "_cel.log";
	system(command.c_str());
	command = "tar rf " + cur_task_id + "_op.tar " + cur_task_id + "_eel.log";
	system(command.c_str());
	command = "tar rf " + cur_task_id + "_op.tar " + cur_task_id + "_stats.log";
	system(command.c_str());
	if(!(eret | cret))
	{
		for(int j = 0; j < tf.task_output_file.size(); j++)
			command = ("tar rf " + cur_task_id + "_op.tar " + tf.task_output_file[j]);
		system(command.c_str());
	}
	command = ("gzip -f " + cur_task_id + "_op.tar");
	system(command.c_str());

	return max(eret,cret);
}
