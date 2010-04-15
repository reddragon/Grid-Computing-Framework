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

#include "parser.h"
#include "config.h"
#include "archiver.h"
#include "command.h"
#include "ping.h"
#include "file_transfer.h"

using namespace std;

XMLFile *xf;
ParsedXMLElements *pxe;

map<int,int> task_status;
int total_tasks_done;

/*
//TODO
void do_topo_sort(ParsedXMLElements *pxe);
*/


void init_task_status()
{
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		task_status[i] = TO_BE_DONE;
}

int get_next_task()
{	
	bool processing_tasks = false;
	for(int i = 0; i < (pxe->Tasks.size()); i++)
		if(task_status[i] == TO_BE_DONE)
			return i;
		else if(task_status[i] == PROCESSING)
			processing_tasks = true;
  
	/*
		//TODO
		If no task can be executed yet,
		return -2

		If no task is available yet, because 
		they are being executed,
		return -3

	*/
	
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
	string selected_client;
	int task;
};


fileinfo rfi;
fileinfo r;
void * coordinate_task(void *arg)
{
	coordinate_task_args *cta = (coordinate_task_args *) arg;
	int task = (cta)->task;
	string selected_client = (cta)->selected_client;
	
	string file_name = pxe->Tasks[task].task_id + ".tar.gz";
	string file_send_command = "COLLECTFILE";
	int file_size = get_file_size(file_name);
	command c, filecoord;
	stringstream ss;
	ss << file_send_command << " " << file_name << " " << file_size; 
	c.command_str = ss.str();
	c.client = selected_client;

	cout << (cta)->selected_client << endl;	
	sem_wait(&sc_sem);
	send_command_to_client(c);
	sem_post(&sc_sem);

	bool collect_file_ack_rcd = false;
	string expected_response = "COLLECTFILEACK "  + file_name;
	for(int i = 0; i < CFRRETRIES; i++)
	{
		sem_wait(&cr_sem);
		for(set<c_resp>::iterator it = client_response.begin(); it != client_response.end(); it++)
		{
			if(it->response == expected_response && it->client == selected_client)
			{
				collect_file_ack_rcd = true;
				client_response.erase(*it);
				break;
			}
		}
		sem_post(&cr_sem);
		if(collect_file_ack_rcd) break;
		usleep(CFRTIMEOUT);
	}
	if(!collect_file_ack_rcd) 
	{
		//TODO
		/*
			A node might send later and be stuck
			Need to take care here. Probably set
			busy = -2 and later when it sends a 
			RESET command, set it to -1
		*/
		sem_wait(&nm_sem);
		node_map[cta->selected_client].busy = -1;
		sem_post(&nm_sem);
		task_status[cta->task] = TO_BE_DONE;
		return NULL;
	}
	cout << "COLLECTFILEACK received for " << file_name << endl;
	
	fileinfo sfi;
	sfi.file_name = file_name;	
	sfi.host_name = selected_client;
	sfi.file_size = file_size;
		
	sem_wait(&sf_sem);
	int send_file_res = send_file(&sfi);
	sem_post(&sf_sem);
	
	if(send_file_res == 2)
	{
		cout << "Client crashed." << endl;
		sem_wait(&nm_sem);
		node_map.erase(cta->selected_client);
		sem_post(&nm_sem);
		task_status[cta->task] = TO_BE_DONE;
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
		for(set<c_resp>::iterator it = client_response.begin(); it != client_response.end(); it++)
		{
			if(it->response.substr(0,expected_response.size()) == expected_response && it->client == selected_client)
			{
				collect_res_str = it->response;
				collect_res_rcd = true;
				client_response.erase(*it);
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
		cout << (cta->selected_client) << " timed out for " << (cta->task) << endl;
		sem_wait(&nm_sem);
		node_map[cta->selected_client].busy = -1;
		sem_post(&nm_sem);
		task_status[cta->task] = TO_BE_DONE;
		return NULL;
	}

	int exp_file_size;
	stringstream fss(collect_res_str);
	string temp, temp1;
	fss >> temp >> temp1 >> exp_file_size;
	
	cout << exp_file_size << endl;
	r.file_name = expected_file;
	r.host_name = selected_client;
	r.file_size = exp_file_size;
	
	sem_wait(&rf_sem);
	int rc_err = receive_file(r);
	sem_post(&rf_sem);
	
	if(!rc_err)
	{
		cout << "File received successfully" << endl;
		task_status[cta->task] = DONE;
	}
	else
	{ 
		cout << "Problem with receiving the file" << endl;
		task_status[cta->task] = TO_BE_DONE;
	}
	sem_wait(&nm_sem);
	node_map[cta->selected_client].busy = -1;
	sem_post(&nm_sem);
	
	
	return NULL;
}

void *start(void *args)
{
	total_tasks_done = 0;
	string pss_file = "inp_ep.in";
	xf = new XMLFile;
	if(xf == NULL) 
	{
		fprintf(stderr, "Failed to allocate memory");
		exit(1);
	}

	pxe = new ParsedXMLElements;
	if(pxe == NULL)
	{
		fprintf(stderr, "Failed to allocate memory");
		exit(1);
	}

	int parse_error = start_to_parse(xf, pxe, pss_file);	     
	if(parse_error)
	{
		fprintf(stderr, "Problem in parsing PSS File. Terminating.\n");
		exit(1);
		
	}
	
	//TODO
	//Finish do_topo_sort
	//do_topo_sort(pxe);
	cout << (pxe->Tasks.size()) << endl;
	init_task_status();
	create_task_archives(xf,pxe);
	
	pthread_t ccresponse_thread;
	pthread_create(&ccresponse_thread, NULL, &(wait_for_client_response_to_command), NULL);
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
			printf("Execution completed.\n");
			break;
		}
		
		string selected_client = "";
		while(1)
		{
			selected_client = load_balancer(next_task);
			if(selected_client != "") break;
			usleep(500000);
		} 
		
		cout << next_task << endl;	
		coordinate_task_args *cta;
		cta = new coordinate_task_args;
		cta->selected_client = selected_client;
		cta->task = next_task;
		task_status[next_task] = PROCESSING;
		sem_wait(&nm_sem);
		node_map[selected_client].busy = 1;
		sem_post(&nm_sem);
		pthread_t cta_thread;
		pthread_create(&cta_thread, NULL, &(coordinate_task), cta);
		
	}
	
	delete xf;
	delete pxe;
	return NULL;
}

int main()
{
	sem_init(&sf_sem, 0, 1);
	sem_init(&rf_sem, 0, 1);
	sem_init(&sc_sem, 0, 1);	
	sem_init(&cr_sem, 0, 1);
	sem_init(&nm_sem, 0, 1);
	
	bind_sf_socket();
	pthread_t ping_thread, listen_to_client_ping_thread, ccstub_thread, start_thread;
	pthread_create(&ping_thread, NULL, &(broadcast_ping), NULL);
	pthread_create(&listen_to_client_ping_thread, NULL , &(listen_to_client_ping), NULL);
	
	pthread_create(&start_thread, NULL, &(start), NULL);
	pthread_join(start_thread,NULL);
	return 0;
}
