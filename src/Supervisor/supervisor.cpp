#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include <set>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>
#include <cmath>
#include <semaphore.h>
#include <csignal>
#include <gtk/gtk.h>

#include "parser.h"
#include "config.h"
#include "archiver.h"
#include "command.h"
#include "ping.h"
#include "file_transfer.h"

#include "interface.h"
#include "callbacks.h"


using namespace std;

XMLFile *xf;
ParsedXMLElements *pxe;

sem_t ts_sem;
map<int,int> task_status;
int total_tasks_done;

int socks;
int xml_selected;

void xml_signal()
{
	xml_selected = 0;
}

char to_send[1000];
void init_task_status()
{
	ofstream check_log;
	check_log.open("checkpoint");
	check_log.close();
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		task_status[i] = TO_BE_DONE;
}

void recover_task_status()
{
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		task_status[i] = TO_BE_DONE;
	ifstream ifs("checkpoint");
	int task;
	while(ifs >> task)
		task_status[task] = DONE;
}

/* This semaphore ensures that if an EMP is executing, other tasks are
 * suspended for the time being */
sem_t emp_sem;
int get_next_task()
{	
	if(pxe->EMP.emp_enabled)
		sem_wait(&emp_sem);
	bool processing_tasks = false;
	
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		if(task_status[i] == TO_BE_DONE)
			{ sem_post(&emp_sem); return i; }
		else if(task_status[i] == PROCESSING)
			processing_tasks = true;
   
	if(pxe->EMP.emp_enabled)
		sem_post(&emp_sem);
	if(processing_tasks == true) 
		return -3;

	return -1;	
}

string load_balancer(int i)
{
	//TODO
	//The 'Genetic Simulated Annealing' algorithm goes here.
	sem_wait(&nm_sem);
	for(map<string,node>::iterator it = node_map.begin(); it != node_map.end(); it++)
	{
		if((it->second).busy == -1)
		{
			sem_post(&nm_sem);
			return it->first;	
		}
	}
	sem_post(&nm_sem);
	return "";
}

struct coordinate_task_args
{
	string selected_worker;
	int task;
};


fileinfo rfi;
fileinfo r;

void * coordinate_task(void *arg)
{
	coordinate_task_args *cta = (coordinate_task_args *) arg;
	int task = (cta)->task;
	string selected_worker = (cta)->selected_worker;

	sprintf(to_send,"Task %d assigned to client %s",task+1,selected_worker.c_str());
	send_to_task_info((string)to_send);	

	string file_name = pxe->Tasks[task].task_id + ".tar.gz";
	
	int port_id = 0;
	while(1)
	{
		int ret = sem_trywait(&sock_sem[port_id]);
		if(!ret) break;
		port_id = (port_id + 1) % socks;
		usleep(500000);
	}
	
	string file_send_command = "COLLECTFILE";
	int file_size = get_file_size(file_name);
	command c, filecoord;
	stringstream ss;
	ss << file_send_command << " " << file_name << " " << file_size << " " << port_id; 
	c.command_str = ss.str();
	c.worker = selected_worker;
	
	sem_wait(&sc_sem);
	send_command_to_worker(c);
	sem_post(&sc_sem);

	bool collect_file_ack_rcd = false;
	string expected_response = "COLLECTFILEACK "  + file_name;
	for(int i = 0; i < CFRRETRIES; i++)
	{
		sem_wait(&cr_sem);
		for(set<c_resp>::iterator it = worker_response.begin(); it != worker_response.end(); it++)
		{
			if(it->response == expected_response && it->worker == selected_worker)
			{
				collect_file_ack_rcd = true;
				worker_response.erase(*it);
				break;
			}
		}
		sem_post(&cr_sem);
		if(collect_file_ack_rcd) break;
		usleep(CFRTIMEOUT);
	}
	if(!collect_file_ack_rcd) 
	{
		sem_post(&sock_sem[port_id]);
		task_status[cta->task] = TO_BE_DONE;
		cout << cta->selected_worker << " not responded, temporarily freezing it." << endl << endl;
		
		sprintf(to_send,"%s not responded, temporarily freezing it.",cta->selected_worker.c_str());
		send_to_overview(to_send);
		
		usleep(3000000);
		
		sem_wait(&nm_sem);
		node_map[cta->selected_worker].busy = -1;
		sem_post(&nm_sem);
		
		return NULL;
	}
	cout << "COLLECTFILEACK received for " << file_name << endl;
	
	fileinfo sfi;
	sfi.file_name = file_name;	
	sfi.host_name = selected_worker;
	sfi.file_size = file_size;
	sfi.port_id = port_id;	
	int send_file_res = send_file(&sfi);
	
	if(send_file_res == 2)
	{
		sem_post(&sock_sem[port_id]);
		cout << "Worker crashed." << endl;
		sprintf(to_send,"\nWorker %s crashed.",selected_worker.c_str());
		send_to_overview(to_send);
		sem_wait(&nm_sem);
		node_map.erase(cta->selected_worker);
		sem_post(&nm_sem);
		task_status[cta->task] = TO_BE_DONE;
		return NULL;
	}
	
	if(send_file_res == 1)
	{
		cout << "Did not receive COLLECTFILEACK from " << cta->selected_worker << endl;
		cout << "Freezing " << cta->selected_worker << " for 5 seconds.." << endl;
		task_status[cta->task] = TO_BE_DONE;
		usleep(5000000);
		sem_post(&sock_sem[port_id]);
		
		sem_wait(&nm_sem);
		node_map[cta->selected_worker].busy = -1;
		sem_post(&nm_sem);
		return NULL;
	}
	
	
	double network_delay = pxe->Tasks[task].network_latency_time;
	double problem_timeout = (pxe->Tasks[task].timeout);
	double time_out = network_delay + problem_timeout;
	int slices = (int)(ceil(time_out/SLICESIZE));

		
	bool collect_res_rcd = false;
	string expected_file = pxe->Tasks[task].task_id + "_op.tar.gz"; 	
	expected_response = "COLLECTRES "  + expected_file;
	string collect_res_str;
	
	int slice_size_usec = (int)ceil(SLICESIZE * 1000000.0);
	double time_taken = 0.0;
	for(int i = 0; i < slices; i++)
	{
		sem_wait(&cr_sem);
		for(set<c_resp>::iterator it = worker_response.begin(); it != worker_response.end(); it++)
		{
			if(it->response.substr(0,expected_response.size()) == expected_response && it->worker == selected_worker)
			{
				collect_res_str = it->response;
				collect_res_rcd = true;
				worker_response.erase(*it);
				break;
			}
		}
		sem_post(&cr_sem);
		if(collect_res_rcd) break;
		time_taken += SLICESIZE;
		if(time_taken > time_out) break;
		usleep(slice_size_usec);
	}
	
	if(collect_res_rcd)
	{
		cout << "Received COLLECTRES for " << task << ", " << collect_res_str << endl;
	}
	else
	{
		sem_post(&sock_sem[port_id]);
		cout << (cta->selected_worker) << " timed out for " << (cta->task) << endl;
		task_status[cta->task] = TO_BE_DONE;
		cout << "Freezing " << cta->task << " for 5 seconds " << endl;
		usleep(5000000);
		sem_wait(&nm_sem);
		node_map[cta->selected_worker].busy = -1;
		sem_post(&nm_sem);
		return NULL;
	}

	int exp_file_size;
	stringstream fss(collect_res_str);
	string temp, temp1;
	fss >> temp >> temp1 >> exp_file_size;
	
	r.file_name = expected_file;
	r.host_name = selected_worker;
	r.file_size = exp_file_size;
	r.port_id = port_id;
	
	int rc_err = receive_file(r);
	
	if(!rc_err)
	{
		sem_post(&sock_sem[port_id]);
		cout << "File received successfully" << endl;
		
		sprintf(to_send,"Received output for task %d from %s\n",cta->task+1,cta->selected_worker.c_str());
		send_to_task_info((string)to_send);
		
		ofstream check_log;
		check_log.open("checkpoint", ios::out | ios::app);
		check_log << cta->task << endl;
		check_log.close();
		cout << "Written in log file" << endl << endl;
		task_status[cta->task] = DONE;
		
		
		if(pxe->EMP.emp_enabled && pxe->Tasks[cta->task].checkpoint) 
		{
			cout << "Arrived at a checkpoint.." << endl;
			send_to_overview("Arrived at a checkpoint..");
			sem_wait(&emp_sem);
			ofstream td_file("tasks_done.emp");
			for(int i = 0; i < (pxe->Tasks.size()); i++)
				if(task_status[i] == DONE)
					td_file << (pxe->Tasks[i].task_id) << endl;
			td_file.close();
			
			for(int i = 0; i < pxe->EMP.emp_execution_commands.size(); i++)
				system((pxe->EMP.emp_execution_commands[i]).c_str());
			
			ifstream er_file("emp_response.emp");
			int commands;
			er_file >> commands;
			for(int j = 0; j < commands; j++)
			{
				string c, arg;
				er_file >> c;
				er_file >> arg;
				cout << "EMP gave the command " << c << " " << arg << endl;
				sprintf(to_send, "EMP gave the command %s %s.\n", c.c_str(), arg.c_str());
				send_to_overview(to_send);
				if(c == "CONTINUE") break;
				if(c == "STOP")
				{
					if(arg == "ALL")
					{
						ofstream check_log;
						check_log.open("checkpoint", ios::out | ios::app);
						for(int i = 0; i < (pxe->Tasks.size()); i++)
						{
							task_status[i] = DONE;
							check_log << i << endl;
						}
						check_log.close();
					}
					else
					{
						ofstream check_log;
						check_log.open("checkpoint", ios::out | ios::app);
						for(int i = 0; i < pxe->Tasks.size(); i++)
							if(pxe->Tasks[i].task_id == arg)
							{
								task_status[i] = DONE;
								check_log << i << endl;
							}
						check_log.close();
					}
				}
				
				if(c == "REDO")
				{
					if(arg == "ALL")
					{
						for(int i = 0; i < (pxe->Tasks.size()); i++)
							task_status[i] = TO_BE_DONE;
					}
					else
					{
						for(int i = 0; i < pxe->Tasks.size(); i++)
							if(pxe->Tasks[i].task_id == arg)
								task_status[i] = TO_BE_DONE;
					}
				}
			}
			er_file.close();
			sem_post(&emp_sem);
			cout << endl;
		}
	}
	else
	{ 
		sem_post(&sock_sem[port_id]);
		cout << "Problem with receiving the file" << endl;
		task_status[cta->task] = TO_BE_DONE;
		cout << "Freezing " << cta->selected_worker << " for 5 seconds.." << endl << endl;
		usleep(5000000);
	}
	
	sem_wait(&nm_sem);
	node_map[cta->selected_worker].busy = -1;
	sem_post(&nm_sem);
	
	return NULL;
}

struct program_arguments {
	bool recover, create_archives, show_parsed_data;
	string pss_file;
};


void * start(void *args)
{
	program_arguments *pa = (program_arguments *)args;
	bool recover = pa->recover;
	total_tasks_done = 0;
	string pss_file = pa->pss_file;
	
	xf = new XMLFile;
	
	if(xf == NULL) 
	{
		fprintf(stderr, "Failed to allocate memory");
		send_to_overview("Failed to allocate memory");
		exit(1);
	}

	pxe = new ParsedXMLElements;
	if(pxe == NULL)
	{
		fprintf(stderr, "Failed to allocate memory");
		send_to_overview("Failed to allocate memory");
		exit(1);
	}

	int parse_error = start_to_parse(xf, pxe, pss_file);
	if(pa->show_parsed_data)
		show_parsed_data(pxe);	     
	
	if(parse_error)
	{
		fprintf(stderr, "Problem in parsing PSS File. Terminating.\n");
		send_to_overview("Problem in parsing PSS File. Terminating.\n");
		exit(1);
	}
	
	if(recover)
		recover_task_status();
	else
		init_task_status();
	
	if(pa->create_archives)
	{
		create_task_archives(xf,pxe);
		send_to_overview("Archives Created.\n");
	}
	cout << "EMP Enabled: " << pxe->EMP.emp_enabled << endl;
	pthread_t ccresponse_thread;
	pthread_create(&ccresponse_thread, NULL, &(wait_for_worker_response_to_command), NULL);
	sf_sock_bound = false;
	while(1)
	{
		int next_task = get_next_task();
		if(next_task == -2 || next_task == -3)
		{
			do
			{
				usleep(500000);
				next_task = get_next_task();
			} while(next_task == -2 || next_task == -3);
		}
		
		if(next_task == -1)
		{	
			printf("\nExecution completed. Closing Sockets\n\n");
			send_to_overview("Execution completed. Closing Sockets\n");
			cout << "RCP Started" << endl;
			send_to_overview("RCP Started\n");
			system(pxe->RCP.rcp_path.c_str());
			cout << "RCP Execution completed" << endl;
			send_to_overview("RCP Execution Completed\n");
			break;
		}
		
		string selected_worker = "";
		while(1)
		{
			selected_worker = load_balancer(next_task);
			if(selected_worker != "") break;
			usleep(500000);
		} 
		
		cout << selected_worker << " chosen for doing " << pxe->Tasks[next_task].task_id << endl;	

		coordinate_task_args *cta;
		cta = new coordinate_task_args;
		cta->selected_worker = selected_worker;
		cta->task = next_task;
		
		task_status[next_task] = PROCESSING;
		
		sem_wait(&nm_sem);
		node_map[selected_worker].busy = 1;
		sem_post(&nm_sem);
		
		pthread_t cta_thread;
		pthread_create(&cta_thread, NULL, &(coordinate_task), cta);
	}
	
	delete xf;
	delete pxe;
}

void * supervisor_starter(void * args)
{	
	int start_send_port = 7500;
	int start_recv_port = 7000;
	
	for(int i = 0; i < 20; i++)
	{ send_ports[i] = start_send_port + i; recv_ports[i] = start_recv_port + i; }
	socks = 10;
	
	xml_selected = 1;
	while(1)
	{
	 if( xml_selected == 1 )
	   g_usleep(1000);
	 else 
		break;
	} 

	gdk_threads_add_timeout( 3000, task_info_timeout , (gpointer)NULL );
    gdk_threads_add_timeout( 1000, overview_timeout, (gpointer)NULL );
    gdk_threads_add_timeout( 1000, worker_timeout, (gpointer)NULL );
	gdk_threads_add_timeout( 1000, current_worker_info_timeout, (gpointer)NULL );

	cout << "Binding Sockets.." << endl;
	send_to_overview("Binding Sockets.." );
	for(int i = 0; i < socks; i++)
		bind_sf_socket(i);
	cout << "Sockets Bound." << endl << endl;
	send_to_overview("Sockets Bound.\n");
	
	for(int i = 0; i < socks; i++)
		sem_init(&sock_sem[i], 0, 1);
	
	sem_init(&sc_sem, 0, 1);	
	sem_init(&cr_sem, 0, 1);
	sem_init(&nm_sem, 0, 1);
	sem_init(&ts_sem, 0, 1);
	sem_init(&emp_sem, 0, 1);
		
	cout << "Starting Ping Threads.." << endl;
	send_to_overview("Starting Ping Threads.." );
	pthread_t ping_thread, listen_to_worker_ping_thread,  ccstub_thread, start_thread;
	pthread_create(&ping_thread, NULL, &(broadcast_ping), NULL);
	pthread_create(&listen_to_worker_ping_thread, NULL , &(listen_to_worker_ping), NULL);
	cout << "Ping threads started." << endl << endl;
	send_to_overview("Ping threads started.\n");
	
    
	program_arguments pa; 
	if(get_status_recover())
	pa.recover = true;
	else 
	pa.recover = false;
	pa.create_archives = true;
	pa.show_parsed_data = false;
	char xml_file_path[100];
	get_xml_path(xml_file_path);
	pa.pss_file = (string)xml_file_path;
	
	cout << "Main thread started." << endl;
	send_to_overview("Main thread started...\n");
	pthread_create(&start_thread, NULL, &(start), &pa);
	pthread_join(start_thread,NULL);	
	
	
}
