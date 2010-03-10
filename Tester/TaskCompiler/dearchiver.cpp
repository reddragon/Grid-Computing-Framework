#include<iostream>
#include<cstdlib>
#include<string>
#include<fstream>

using namespace std;

struct Task
{
	string task_id;
	int priority;
	float timeout;
	string task_compile_command;
	string task_execution_command;
};

int main()
{
	Task t;
	string tid = "t1";
	string command = "gunzip " + tid + ".tar"; 
	
	system(command.c_str());
	
	command = "tar xf " + tid + ".tar";
	system(command.c_str());
	
	ifstream ifs;
	ifs.open( (tid + ".tdd").c_str());
	getline(ifs,t.task_id);
	getline(ifs,t.task_compile_command);
	getline(ifs,t.task_execution_command);
	ifs >> t.priority;
	ifs >> t.timeout;

	return 0;
}
