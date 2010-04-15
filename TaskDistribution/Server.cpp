#include<iostream>
#include<pthread.h>
#include<string>
#include<fstream>
#include "serverlistener.cpp"
#include "parser.cpp"


#define PORT3 "4952"


using namespace std;

XMLFile *xf;
ParsedXMLElements *pxe;

void make_task_archives()
{
	//cout << pxe->Tasks.size() << endl;
	
	for(int i = 0; i < (pxe->Tasks.size()); i++)
	{

		//cout << i << endl;
		string cur_task_id = pxe->Tasks[i].task_id;
		ofstream task_desc_file;
		task_desc_file.open( ((cur_task_id)+".tdd").c_str());
		task_desc_file << cur_task_id << endl;
		task_desc_file << (pxe->Tasks[i].task_compile_command) << endl;
		task_desc_file << (pxe->Tasks[i].task_execution_command) << endl;
		task_desc_file << (pxe->Tasks[i].priority) << endl;
		task_desc_file << (pxe->Tasks[i].timeout) << endl;

		task_desc_file.close();
			
		string command = ("tar cf " + cur_task_id + ".tar " + pxe->Tasks[i].task_source_path);
		system(command.c_str());
		command = "tar rf " + cur_task_id + ".tar " + pxe->Tasks[i].task_inputset_path;
		system(command.c_str());
		command = "tar rf " + cur_task_id + ".tar " + cur_task_id + ".tdd";
		system(command.c_str());
		command = ("gzip -f " + cur_task_id + ".tar");
		system(command.c_str());
		
	}

}

void * parse_file(void * file)
{
	int * result = new int;
	xf = new XMLFile;
	if(xf == NULL) { *result = 1; return result; }
	
	pxe = new ParsedXMLElements;

	strcpy(xf->file_address, ((string *)file)->c_str());
	get_file(xf);
	//show_file(xf);
		
	int parse_result = parse(xf,pxe);
	

	if(parse_result == 0)
	{
		cout << "Making Task Archives\n" << endl;
		make_task_archives();
	}
	else
	{ *result = 1; return result; }
}

vector<string> topo_sorted_tasks;
set<string> tasks_left;
map<string,int> location;
map<int,string> task_id;
vector< set<string> > dep;

void topo_sort()
{
	int task_num = pxe->Tasks.size();
	int i;

	for(i = 0; i < task_num; i++)
	{
	   location[pxe->Tasks[i].task_id] = i;
	   task_id[i] = pxe->Tasks[i].task_id;
	   tasks_left.insert(pxe->Tasks[i].task_id);
	   
	   set<string> deps;
	   for(int j = 0; j < (pxe->Tasks[i].dependencies.size()); j++)
		   	deps.insert(pxe->Tasks[i].dependencies[j]);
	   dep.push_back(deps);
	}

	while(tasks_left.size() > 0)
	{
		string t = "";
		for(set<string>::iterator it = tasks_left.begin(); it != tasks_left.end(); it++)
			if(dep[location[*it]].size() == 0)
			{ t = *it; break; }
		tasks_left.erase(t);
		topo_sorted_tasks.push_back(t);
		for(set<string>::iterator it = tasks_left.begin(); it != tasks_left.end(); it++)
			if(dep[location[*it]].find(t) != dep[location[*it]].end())
				dep[location[*it]].erase(t);
	}

	/*for(i = 0; i < topo_sorted_tasks.size(); i++)
		cout << topo_sorted_tasks[i] << " " << i << endl;*/


}


bool busy[MAXHOSTS];
int last_client;

int load_balancer(string s)
{
	//TODO
	/* Proper Load balancing algorithm to be added */
	for(int i = (last_client + 1) % MAXHOSTS; ; i = (i + 1) % MAXHOSTS)
	{
		if(!busy[i] && host_list[i].last_seen != -1)
			return i;
	}
}

int initiate_task_sending(int chosen_host, string t)
{
    printf("Asking host %s, to do task %s\n", host_list[chosen_host].host_name, t.c_str());
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, numbytes;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(host_list[chosen_host].host_name,PORT3, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "Error: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
	{
            perror("Error: Could not create socket\n");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Error: Failed to bind socket\n");
        return 1;
    }
	
    if ((numbytes = sendto(sockfd, t.c_str(), strlen(t.c_str()), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("Error: Message could not be sent\n");
        return 1;
    }
    freeaddrinfo(servinfo);
    //printf("Successfully sent the resonse message: %s, to destination: %s\n", r->message, r->host_name);
	close(sockfd);
    return 0;

}

void * start_distribution(void * args)
{

	topo_sort();
	int i;
	//TODO
	/* Dependencies to be handled. */
	memset(busy, 0, sizeof(busy));
	last_client = -1;

	for(i = 0; i < topo_sorted_tasks.size(); i++)
	{
		last_client = load_balancer(topo_sorted_tasks[i]);
		cout << topo_sorted_tasks[i] << " assigned to: " << host_list[last_client].host_name << endl; 
		busy[last_client] = 1;
		int res = 1;
		while(res)
		  res = initiate_task_sending(last_client, topo_sorted_tasks[i]);
		
	}

}

int main()
{
	string file = "MergeSort.pss";
	pthread_t conn_thread, parse_thread, dist_thread;
	pthread_create(&conn_thread, NULL, &(start_server), NULL);
	pthread_create(&parse_thread, NULL, &(parse_file), (void *)(&file));
	
	pthread_create(&dist_thread, NULL, &(start_distribution), NULL);
	pthread_join(dist_thread,NULL);		
	pthread_join(parse_thread, NULL);
	pthread_join(conn_thread, NULL);	
	return 0;
}
