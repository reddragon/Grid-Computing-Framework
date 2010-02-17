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
	
	/*
	cout << t.task_id << endl;
	cout << t.task_compile_command << endl;
	cout << t.task_execution_command << endl;
	cout << t.priority << endl;
	cout << t.timeout << endl;
	*/

	system(t.task_compile_command.c_str());
	system((t.task_execution_command + " < " + t.task_id + "_inp.inp > " + t.task_id + "_out.out").c_str());


	return 0;
}
