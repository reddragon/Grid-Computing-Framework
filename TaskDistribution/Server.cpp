#include<iostream>
#include<pthread.h>
#include<string>
#include<fstream>
#include "serverlistener.cpp"
#include "parser.cpp"

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

	for(i = 0; i < topo_sorted_tasks.size(); i++)
		cout << topo_sorted_tasks[i] << " " << i << endl;


}

void * start_distribution(void * args)
{
	topo_sort();	
}

int main()
{
	string file = "MergeSort.pss";
	pthread_t conn_thread, parse_thread, dist_thread;
	pthread_create(&conn_thread, NULL, &(start_server), NULL);
	pthread_create(&parse_thread, NULL, &(parse_file), (void *)(&file));
	pthread_join(parse_thread, NULL);
	
	pthread_create(&dist_thread, NULL, &(start_distribution), NULL);
	
	
	pthread_join(conn_thread, NULL);	
	return 0;
}
