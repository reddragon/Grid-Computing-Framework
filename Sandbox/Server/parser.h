/* Parses the XML File
 * The XML File contains the Problem Solving Schema
 * */

#include <cstdio>
#include <vector>
#include <cstring>
#include <malloc.h>
#include <iostream>
#include <sstream>
#include <set>
#include <cstring>
#include <map>

#include "config.h"

using namespace std;

typedef struct XMLFile
{
	char file_address[MAXPATHLENGTH];
	char file[MAXFILESIZE];
	int file_length;
}XMLFile;

typedef struct ProblemDescripton
{
	string name; 
	string user; 
	string problem_id;
	string purpose;
}ProblemDescription;


typedef struct UnitTask
{
	string task_id;
    	short priority;
	vector<string> dependencies;
	double timeout;
	double network_latency_time;
	vector<string> task_source_path;
	vector<string> task_inputset_path;
	vector<string> task_outputset_path;
	vector<string> task_compile_command;
	string task_execution_command;
} UnitTask;

typedef struct ResultCompiler
{
	string rcp_path;
}ResultCompiler;

typedef struct ExecutionMonitor
{
	double emp_timeout;
	vector<string> checkpoints;
	vector<string> emp_execution_commands;

}ExecutionMonitor;

typedef struct ParsedXMLElements
{
	ProblemDescription Description;
	vector<UnitTask> Tasks;
	ResultCompiler RCP;
	ExecutionMonitor EMP;
}ParsedXMLElements;

void get_file(XMLFile *xf)
{
	FILE *fp;
	char ch;
	fp = fopen(xf->file_address,"r");
	xf->file_length = 0;
	while(1)
	{
		ch = fgetc(fp);
		if(ch == '\n' || ch == '\t') ch = ' ';
		if(ch == EOF) break;
		xf->file[xf->file_length++] = ch;
	}
	xf->file[xf->file_length] = '\0';
	xf->file_length++;
}



int show_parsed_data(ParsedXMLElements *pxe)
{
	cout<<"Problem Name:\t" << pxe->Description.name << endl;
	cout<<"User:\t" << pxe->Description.user << endl;
	cout<<"Problem Description:\t" << pxe->Description.problem_id << endl;
	cout<<"Problem Purpose:\t" << pxe->Description.purpose << endl;

	for(int i = 0; i < (pxe->Tasks.size()); i++)
	{
		cout<<"Task Id:\t" << pxe->Tasks[i].task_id << endl;
		cout<<"Task Priority:\t" << pxe->Tasks[i].priority << endl;
		cout<<"Task Timeout:\t" << pxe->Tasks[i].timeout << endl;
		cout<<"Network Latency Time:\t" << pxe->Tasks[i].network_latency_time << endl;
		cout<<"Dependencies: ";
		for(int j = 0; j < (pxe->Tasks[i].dependencies.size()); j++)
			cout << pxe->Tasks[i].dependencies[j] << "\t";
		cout << endl;

		cout<<"Task Source Path:\t" << pxe->Tasks[i].task_source_path.size() << endl;
		for(int j = 0; j < pxe->Tasks[i].task_source_path.size(); j++)
		{
			cout << pxe->Tasks[i].task_source_path[j] << endl;
		}
		cout<<"Task Input Set Path:\t" << pxe->Tasks[i].task_inputset_path.size() << endl;
		for(int j = 0; j < pxe->Tasks[i].task_inputset_path.size(); j++)
		{
			cout << pxe->Tasks[i].task_inputset_path[j] << endl;
		}
		cout<<"Task Output Set Path:\t" << pxe->Tasks[i].task_outputset_path.size() << endl;
		for(int j = 0; j < pxe->Tasks[i].task_outputset_path.size(); j++)
		{
			cout << pxe->Tasks[i].task_outputset_path[j] << endl;
		}
		cout<<"Task Compile Command:\t" << pxe->Tasks[i].task_compile_command.size() << endl;
		for(int j = 0; j < pxe->Tasks[i].task_compile_command.size(); j++)
		{
			cout << pxe->Tasks[i].task_compile_command[j] << endl;
		}
		cout<<"Task Execution Command:\t" << pxe->Tasks[i].task_execution_command << endl << endl;	
	}

	cout << "RCP Path:\t" << pxe->RCP.rcp_path << endl;
	cout << "EMP: " << endl;
	cout << "EMP Timeout: " << pxe->EMP.emp_timeout << endl;
	cout << "EMP Checkpoints: " << endl;
	for(int i = 0; i < pxe->EMP.checkpoints.size(); i++)
	{
		cout << "\t" << pxe->EMP.checkpoints[i] << endl;
	}

	cout << "EMP execution commands: " << endl;	
	for(int i = 0; i < pxe->EMP.emp_execution_commands.size(); i++)
	{
		cout << "\t" << pxe->EMP.emp_execution_commands[i] << endl;
	}

	return 0;
}



int report_validation_error(short error_number)
{
	char error[200];
	switch(error_number)
	{
		case 1: strcpy(error, "Problem name missing."); break;
		case 2: strcpy(error, "Problem name longer than 100 characters."); break;
		case 3: strcpy(error, "User name missing."); break;
		case 4: strcpy(error, "User name longer than 50 characters."); break;
		case 5: strcpy(error, "Problem id missing."); break;
		case 6: strcpy(error, "Problem id longer than 50 characters."); break;
		case 7: strcpy(error, "Problem purpose missing."); break;
		case 8: strcpy(error, "Problem purpose longer than 400 characters."); break;
		case 9: strcpy(error, "Task id missing."); break;
		case 10: strcpy(error, "Task id longer than 50 characters."); break;
		case 11: strcpy(error, "Task priority missing, or not in the range of 1-100."); break;
		case 12: strcpy(error, "Task timeout missing or not in the range of 0.1 - 1000000000."); break;
		case 13: strcpy(error, "Task source path missing."); break;
		case 14: strcpy(error, "Task source path longer than 300 characters."); break;
		case 15: strcpy(error, "Task input path missing."); break;
		case 16: strcpy(error, "Task input path longer than 300 characters."); break;
		case 17: strcpy(error, "Task compile command missing."); break;
		case 18: strcpy(error, "Task compile command longer than 300 characters."); break;
		case 19: strcpy(error, "Task execution command missing."); break;
		case 20: strcpy(error, "Task execution command longer than 300 characters."); break;	
		case 21: strcpy(error, "Task output set path missing."); break;
		case 22: strcpy(error, "Task outputset longer than 300 characters."); break;
		case 23: strcpy(error, "Task depends on a task, which has not been defined."); break;
		case 24: strcpy(error, "Deadlock detected in the dependency matrix."); break;
		case 25: strcpy(error, "Network Latency Time missing or not in the range of 0.1 - 1000000000."); break;
		case 26: strcpy(error, "EMP Timeout missing or not in the range of 0.1 - 10000."); break;
		case 27: strcpy(error, "Invalid checkpoint."); break;

	}
	printf("%s\n",error);
	return 1;
}


int validate_parsed_data(ParsedXMLElements *pxe)
{
		//TODO
		/* Validation of files and commands left 
		   Stripping of leading and following spaces to be done in all fields.
		*/
		//show_parsed_data(pxe);
			
		map<string,int> taskid_index;

		int i, j, k, n = 0;
		int mat[1000][1000];
		memset(mat, 0, sizeof(mat));

		for(i = 0; i < pxe->Tasks.size(); i++)
		{
			taskid_index[pxe->Tasks[i].task_id] = i+1;
			n = i+1;
		}

		for(i = 0; i < pxe->Tasks.size(); i++)
		{
			int ti = taskid_index[pxe->Tasks[i].task_id];
			for(j = 0; j < pxe->Tasks[i].dependencies.size(); j++)
			{
				if(taskid_index.find(pxe->Tasks[i].dependencies[j]) == taskid_index.end())
				{
					return report_validation_error(23);
				}

				
				mat[ti][taskid_index[pxe->Tasks[i].dependencies[j]]] = 1;

			 }
		}	

		for(i = 1; i <= n; i++)
			if(mat[i][j])
		   for(j = 1; j <= n; j++)
			for(k = 1; k <= n; k++)
				if(mat[i][j] == 1 && mat[j][k]== 1)
				mat[i][k] = 1;
		
		for(i = 0; i < pxe->EMP.checkpoints.size(); i++)
		{
			if(taskid_index.find(pxe->EMP.checkpoints[i]) == taskid_index.end())
				return report_validation_error(27);
		}	 
		
		if(pxe->EMP.emp_timeout < 0.1 || pxe->EMP.emp_timeout > 10000)
		return report_validation_error(26);

		for(i = 1; i <= n; i++)
			if(mat[i][i])
				return report_validation_error(24);

	printf("XML Schema parsed and validated successfully\n");
	return 0;
		
}


int i;


int report_parse_error(short error_number)
{
	char error[200];
	switch(error_number)
	{
		case 1: strcpy(error, "Improperly formed or missing opening problem tag."); 
				break;
		case 2: strcpy(error, "Improperly formed or missing closing problem tag.");
				break;
		case 3: strcpy(error, "Unexpected characters found after closing problem tag.");
				break;
		case 4: strcpy(error, "Improperly formed or missing opening description tag. ");
				break;
		case 5: strcpy(error, "Improperly formed or missing closing description tag.");
				break;
		case 6: strcpy(error, "Improperly formed or missing opening name tag.");
				break;
		case 7: strcpy(error, "Improperly formed or missing closing name tag.");
				break;
		case 8: strcpy(error, "Improperly formed or missing opening problem id tag.");
				break;
		case 9: strcpy(error, "Improperly formed or missing closing problem id tag.");
				break;
		case 10: strcpy(error, "Improperly formed or missing opening user id tag.");
				break;
		case 11: strcpy(error, "Improperly formed or missing closing user id tag.");
				break;
		case 12: strcpy(error, "Improperly formed or missing opening purpose id tag.");
				break;
		case 13: strcpy(error, "Improperly formed or missing closing purpose id tag.");
				break;
		case 14: strcpy(error, "Improperly formed or missing opening tasks tag.");
				break;
		case 15: strcpy(error, "Improperly formed or missing closing tasks tag.");
				break;
		case 16: strcpy(error, "Improperly formed or missing opening task tag.");
				break;
		case 17: strcpy(error, "Improperly formed or missing closing task tag.");
				break;
		case 18: strcpy(error, "Improperly formed or missing opening task id tag.");
				break;
		case 19: strcpy(error, "Improperly formed or missing closing task id tag.");
				break;
		case 20: strcpy(error, "Improperly formed or missing opening task priority tag.");
				break;
		case 21: strcpy(error, "Improperly formed or missing closing task priority tag.");
				break;
		case 22: strcpy(error, "Improperly formed or missing opening dependencies tag.");
				break;
		case 23: strcpy(error, "Improperly formed or missing closing dependencies tag.");
				break;
		case 24: strcpy(error, "Improperly formed or missing opening do tag.");
				break;
		case 25: strcpy(error, "Improperly formed or missing closing do tag.");
				break;
		case 26: strcpy(error, "Improperly formed or missing opening task timeout tag.");
				break;
		case 27: strcpy(error, "Improperly formed or missing closing task timeout tag.");
				break;
		case 28: strcpy(error, "Improperly formed or missing opening task source path tag.");
				break;
		case 29: strcpy(error, "Improperly formed or missing closing task source path tag.");
				break;
		case 30: strcpy(error, "Improperly formed or missing opening task compile command tag.");
				break;
		case 31: strcpy(error, "Improperly formed or missing closing task compile command tag.");
				break;
		case 32: strcpy(error, "Improperly formed or missing opening task execution command tag.");
				break;
		case 33: strcpy(error, "Improperly formed or missing closing task executon command tag.");
				break;
		case 34: strcpy(error, "Improperly formed or missing opening rcp tag.");
				break;
		case 35: strcpy(error, "Improperly formed or missing closing rcp tag.");
				break;
		case 36: strcpy(error, "Improperly formed or missing opening rcp source path tag.");
				break;
		case 37: strcpy(error, "Improperly formed or missing closing rcp source path tag.");
				break;
		case 38: strcpy(error, "Improperly formed or missing opening emp tag.");
				break;
		case 39: strcpy(error, "Improperly formed or missing closing emp tag.");
				break;
		case 40: strcpy(error, "Improperly formed or missing opening emp time out tag.");
				break;
		case 41: strcpy(error, "Improperly formed or missing closing emp time out tag.");
				break;
		case 42: strcpy(error, "Improperly formed or missing opening task input set path tag.");
				break;
		case 43: strcpy(error, "Improperly formed or missing closing task input set path tag.");
				break;
		case 44: strcpy(error, "Improperly formed or missing opening task output set path tag.");
				break;		
		case 45: strcpy(error, "Improperly formed or missing closing task output set path tag.");
				break;
		case 46: strcpy(error, "Improperly formed or missing opening network latency time tag.");
				break;
		case 47: strcpy(error, "Improperly formed or missing closing network latency time tag.");
				break;
		case 48: strcpy(error, "Improperly formed or missing opening ts tag.");
				break;
		case 49: strcpy(error, "Improperly formed or missing closing ts tag.");
				break;
		case 50: strcpy(error, "Improperly formed or missing opening ti tag.");
				break;
		case 51: strcpy(error, "Improperly formed or missing closing ti tag.");
				break;
		case 52: strcpy(error, "Improperly formed or missing opening to tag.");
				break;
		case 53: strcpy(error, "Improperly formed or missing closing to tag.");
				break;
		case 54: strcpy(error, "Improperly formed or missing opening tc tag.");
				break;
		case 55: strcpy(error, "Improperly formed or missing closing tc tag.");
				break;
		case 56: strcpy(error, "Improperly formed or missing opening checkpoints tag.");
				break;
		case 57: strcpy(error, "Improperly formed or missing closing checkpoints tag.");
				break;
		case 58: strcpy(error, "Improperly formed or missing opening cp tag.");
				break;
		case 59: strcpy(error, "Improperly formed or missing closing cp tag.");
				break;
		case 60: strcpy(error, "Improperly formed or missing opening emp execution commands tag.");
				break;
		case 61: strcpy(error, "Improperly formed or missing closing emp execution commands tag.");
				break;
		case 62: strcpy(error, "Improperly formed or missing opening ec tag.");
				break;
		case 63: strcpy(error, "Improperly formed or missing closing ec tag.");
				break;


	}

	printf("%s\n", error);
	printf("\nAt character: %d\n", i);
	return 1;
}

int parse(XMLFile *xf, ParsedXMLElements *pxe)
{
	int j;

	/*Tags*/
	char problem_tag[30]; int problem_tag_length;
	char description_tag[30]; int description_tag_length;
	char name_tag[30]; int name_tag_length;
	char problemid_tag[30]; int problemid_tag_length;
	char userid_tag[30]; int userid_tag_length;
	char purpose_tag[30]; int purpose_tag_length;
	char tasks_tag[30]; int tasks_tag_length;
	char task_tag[30]; int task_tag_length;
	char taskid_tag[30]; int taskid_tag_length;
	char taskpriority_tag[30]; int taskpriority_tag_length;
	char dependencies_tag[30]; int dependencies_tag_length;
	char do_tag[30]; int do_tag_length;
	char tasktimeout_tag[30]; int tasktimeout_tag_length;
	char networklatencytime_tag[30]; int networklatencytime_tag_length;
	char tasksourcepath_tag[30]; int tasksourcepath_tag_length;
	char ts_tag[30]; int ts_tag_length;
	char taskinputsetpath_tag[30]; int taskinputsetpath_tag_length;
	char ti_tag[30]; int ti_tag_length;
	char taskcompilecommand_tag[30]; int taskcompilecommand_tag_length;
	char tc_tag[30]; int tc_tag_length;
	char taskexecutioncommand_tag[30]; int taskexecutioncommand_tag_length;
	char rcp_tag[30]; int rcp_tag_length;
	char rcpsourcepath_tag[30]; int rcpsourcepath_tag_length;
	char emp_tag[30]; int emp_tag_length;
	char emptimeout_tag[30]; int emptimeout_tag_length;
	char taskoutputsetpath_tag[30]; int taskoutputsetpath_tag_length;
	char to_tag[30]; int to_tag_length;
	char checkpoints_tag[30]; int checkpoints_tag_length;
	char cp_tag[30]; int cp_tag_length;
	char executioncommands_tag[30]; int executioncommands_tag_length;
	char ec_tag[30]; int ec_tag_length;


	/*Set Tags*/
	strcpy(problem_tag, "problem"); problem_tag_length = strlen(problem_tag);
	strcpy(description_tag, "description"); description_tag_length = strlen(description_tag);
	strcpy(name_tag, "name"); name_tag_length = strlen(name_tag);
	strcpy(problemid_tag, "problemid"); problemid_tag_length = strlen(problemid_tag);
	strcpy(userid_tag, "userid"); userid_tag_length = strlen(userid_tag);
	strcpy(purpose_tag, "purpose"); purpose_tag_length = strlen(purpose_tag);
	strcpy(tasks_tag, "tasks"); tasks_tag_length = strlen(tasks_tag);
	strcpy(task_tag, "task"); task_tag_length = strlen(task_tag);
	strcpy(taskid_tag, "taskid"); taskid_tag_length = strlen(taskid_tag);
	strcpy(taskpriority_tag, "taskpriority"); taskpriority_tag_length = strlen(taskpriority_tag);
	strcpy(dependencies_tag, "dependencies"); dependencies_tag_length = strlen(dependencies_tag);
	strcpy(do_tag, "do"); do_tag_length = strlen(do_tag);
	strcpy(tasktimeout_tag, "tasktimeout"); tasktimeout_tag_length = strlen(tasktimeout_tag);
	strcpy(networklatencytime_tag, "networklatencytime"); networklatencytime_tag_length = strlen(networklatencytime_tag);
	strcpy(tasksourcepath_tag, "tasksourcepath"); tasksourcepath_tag_length = strlen(tasksourcepath_tag);
	strcpy(ts_tag, "ts"); ts_tag_length = strlen(ts_tag);
	strcpy(taskinputsetpath_tag, "taskinputsetpath"); taskinputsetpath_tag_length = strlen(taskinputsetpath_tag);
	strcpy(ti_tag, "ti"); ti_tag_length = strlen(ti_tag);
	strcpy(taskcompilecommand_tag, "taskcompilecommand"); taskcompilecommand_tag_length = strlen(taskcompilecommand_tag);
	strcpy(tc_tag, "tc"); tc_tag_length = strlen(tc_tag);
	strcpy(taskexecutioncommand_tag, "taskexecutioncommand"); taskexecutioncommand_tag_length = strlen(taskexecutioncommand_tag);
	strcpy(rcp_tag, "rcp"); rcp_tag_length = strlen(rcp_tag);
	strcpy(rcpsourcepath_tag, "rcpsourcepath"); rcpsourcepath_tag_length = strlen(rcpsourcepath_tag);
	strcpy(emp_tag, "emp"); emp_tag_length = strlen(emp_tag);
	strcpy(emptimeout_tag, "emptimeout"); emptimeout_tag_length = strlen(emptimeout_tag);	
	strcpy(taskoutputsetpath_tag, "taskoutputsetpath"); taskoutputsetpath_tag_length = strlen(taskoutputsetpath_tag);
	strcpy(to_tag, "to"); to_tag_length = strlen(to_tag);
	strcpy(checkpoints_tag, "checkpoints"); checkpoints_tag_length = strlen(checkpoints_tag);
	strcpy(cp_tag, "cp"); cp_tag_length = strlen(cp_tag);
	strcpy(executioncommands_tag, "executioncommands"); executioncommands_tag_length = strlen(executioncommands_tag);
	strcpy(ec_tag, "ec"); ec_tag_length = strlen(ec_tag);


	/* <problem> */
	for(i = 0; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break; 
	
	//i += 1;
	if(i >= xf->file_length) return report_parse_error(1);
	
	if(xf->file[i] == '<' && xf->file[i+problem_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+problem_tag_length); j++)
				if(problem_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(1);
	}
	else return report_parse_error(1);
	i += problem_tag_length + 2;
	/* <problem> */
	

	/* <description> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	
	if(i >= xf->file_length) return report_parse_error(4);

	if(xf->file[i] == '<' && xf->file[i+description_tag_length+1] == '>')
	{
			
			for(j = i+1; j < (i+1+description_tag_length); j++)
				if(description_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(4);
	}
	else return report_parse_error(4);
	i += description_tag_length + 2;
	
	/* <description> */
	
        /* <name> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(6);
	
	if(xf->file[i] == '<' && xf->file[i+name_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+name_tag_length); j++)
				if(name_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(6);
	}
	else return report_parse_error(6);
	i += name_tag_length + 2;
	/* <name> */

	
	/* </name> */
	
	pxe->Description.name = "";
	//(pxe.Description).name = "";
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
		else
		{
			if(pxe->Description.name.size() > 100)
				return report_validation_error(2); 
			
			
			pxe->Description.name.push_back(xf->file[i]);
		}
		if(pxe->Description.name.size() == 0)
				return report_validation_error(1);



	if(i >= xf->file_length) return report_parse_error(7);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+name_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+name_tag_length); j++)
				if(name_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(2);
	}
	else return report_parse_error(7);
	i += name_tag_length + 3;
	
	/* </name> */

	/* <problemid> */
		for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
	
	
	if(i >= xf->file_length) return report_parse_error(8);
	
	if(xf->file[i] == '<' && xf->file[i+problemid_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+problemid_tag_length); j++)
				if(problemid_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(8);
	}
	else return report_parse_error(8);
	i += problemid_tag_length + 2;
	/* <problemid> */

	/* </problemid> */
	
		pxe->Description.problem_id = "";
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
		else
		{
			if(pxe->Description.problem_id.size() > 50)
				return report_validation_error(6);
			else 
			pxe->Description.problem_id.push_back(xf->file[i]);
		}
	if(pxe->Description.problem_id.size() == 0) return report_validation_error(5);

		
	if(i >= xf->file_length) return report_parse_error(9);
	
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+problemid_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+problemid_tag_length); j++)
				if(problemid_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(9);
	}
	else return report_parse_error(9);
	i += problemid_tag_length + 3;
	
	/* </problemid> */
	
	/* <user> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
	
	

	if(i >= xf->file_length) return report_parse_error(10);
	
	if(xf->file[i] == '<' && xf->file[i+userid_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+userid_tag_length); j++)
				if(userid_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(10);
	}
	else return report_parse_error(10);
	i += userid_tag_length + 2;
	/* <user> */

	/* </user> */
	pxe->Description.user = "";
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
		else
		{
			if(pxe->Description.user.size() > 50)
				return report_validation_error(4);
			else 
			pxe->Description.user.push_back(xf->file[i]);
		}

	if(pxe->Description.user.size() == 0) return report_validation_error(3);
		
	if(i >= xf->file_length) return report_parse_error(11);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+userid_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+userid_tag_length); j++)
				if(userid_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(11);
	}
	else return report_parse_error(11);
	i += userid_tag_length + 3;
	/* </user> */

	/* <purpose> */
	
	pxe->Description.purpose = "";
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(12);
	
	if(xf->file[i] == '<' && xf->file[i+purpose_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+purpose_tag_length); j++)
				if(purpose_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(12);
	}
	else return report_parse_error(12);
	i += purpose_tag_length + 2;
	/* <purpose> */

	/* </purpose> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
		else
		{
			if(pxe->Description.purpose.size() > 400)
				return report_validation_error(8);
			else 
				pxe->Description.purpose.push_back(xf->file[i]);
		}

		if(pxe->Description.purpose.size() == 0)
				return report_validation_error(7);
		
	if(i >= xf->file_length) return report_parse_error(13);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+purpose_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+purpose_tag_length); j++)
				if(purpose_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(13);
	}
	else return report_parse_error(13);
	i += purpose_tag_length + 3;
	
	/* </purpose> */

	/* </description> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(5);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+description_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+description_tag_length); j++)
				if(description_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(2);
	}
	else return report_parse_error(5);
	i += description_tag_length + 3;
	/* </description> */
	
	/* <tasks> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(14);
	
	if(xf->file[i] == '<' && xf->file[i+tasks_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+tasks_tag_length); j++)
				if(tasks_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(14);
	}
	else return report_parse_error(14);
	i += tasks_tag_length + 2;
	/* <tasks> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
	
	int tot_task_tags = 0;
	int cur_index = i;
	int task_tag_set = 1;
	
	
	while(task_tag_set)
	{
		if(xf->file[cur_index] == '<' && xf->file[cur_index+task_tag_length+1] == '>')
		{
				for(j = cur_index+1; j < (cur_index+1+task_tag_length); j++)
					if(task_tag[j-(cur_index+1)] != xf->file[j])
						{ 
						task_tag_set = 0; break; }
					
				if(task_tag_set == 0) break;
				tot_task_tags += 1;

				/* <task> */
				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(16);
	
				if(xf->file[i] == '<' && xf->file[i+task_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+task_tag_length); j++)
					if(task_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(16);
				}
				else return report_parse_error(16);
				i += task_tag_length + 2;
				/* <task> */	
				UnitTask ut;	
				
				/* <taskid> */
				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(18);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+taskid_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+taskid_tag_length); j++)
					if(taskid_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(18);
				}
				else return report_parse_error(18);
				i += taskid_tag_length + 2;
				/* <task> */	
					
				/* </taskid> */
					ut.task_id = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(ut.task_id.size() > 50)
						return report_validation_error(10); 
					
					ut.task_id.push_back(xf->file[i]);
				}
				if(ut.task_id.size() == 0)
						return report_validation_error(9);
				
				if(i >= xf->file_length) return report_parse_error(19);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+taskid_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+taskid_tag_length); j++)
						if(taskid_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(19);
				}
				else return report_parse_error(19);
				i += taskid_tag_length + 3;
	
				/* </taskid> */
				
				/* <taskpriority> */
				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(20);
	
				if(xf->file[i] == '<' && xf->file[i+taskpriority_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+taskpriority_tag_length); j++)
					if(taskpriority_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(20);
				}
				else return report_parse_error(20);
				i += taskpriority_tag_length + 2;
				/* <taskpriority> */	
					
				/* </taskpriority> */
				ut.priority = 0;
				string temp_str = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{	
					temp_str.push_back(xf->file[i]);
				}

				stringstream ss_tp(temp_str);
				ss_tp >> ut.priority;

				if(!(ut.priority > 0 && ut.priority <= 100))
				{
					return report_validation_error(11);
				}
		
				if(i >= xf->file_length) return report_parse_error(21);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+taskpriority_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+taskpriority_tag_length); j++)
						if(taskpriority_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(21);
				}
				else return report_parse_error(21);
				i += taskpriority_tag_length + 3;

				/* </taskpriority> */
				
				/* <dependencies> */
				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(22);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+dependencies_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+dependencies_tag_length); j++)
					if(dependencies_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(22);
				}
				else return report_parse_error(22);
				i += dependencies_tag_length + 2;
				/* <dependencies> */	
				
				int do_index = i;
				int do_tag_set = 1;
				int tot_do_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				do_index = i;
				while(do_tag_set)
				{
					if(xf->file[do_index] == '<' && xf->file[do_index+do_tag_length+1] == '>')
					{
						for(j = do_index+1; j < (do_index+1+do_tag_length); j++)
						if(do_tag[j-(do_index+1)] != xf->file[j])
						{ 
							do_tag_set = 0; break; 
						}
					
						if(do_tag_set == 0) break;
					
					tot_do_tags += 1;

			          /* <do> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(24);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+do_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+do_tag_length); j++)
					if(do_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(24);
				}
				else return report_parse_error(24);
				i += do_tag_length + 2;
				/* <do> */	
					
				/* </do> */
				string temp_dep = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_dep.size() > 40)
					{	//return validate_error(1); 
					}
			
					temp_dep.push_back(xf->file[i]);
				}
				if(temp_dep !=  "") ut.dependencies.push_back(temp_dep);
		
				if(i >= xf->file_length) return report_parse_error(25);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+do_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+do_tag_length); j++)
						if(do_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(25);
				}
				else return report_parse_error(25);
				i += do_tag_length + 3;
	
				/* </do> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						do_index = i;
					}
					else break;
				}
					
				
				/* </dependencies> */
				for(; i < xf->file_length; i++)
					if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(23);	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+dependencies_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+dependencies_tag_length); j++)
						if(dependencies_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(23);
				}
				else return report_parse_error(23);
				i += dependencies_tag_length + 3;
	
				/* </dependencies> */
					
				 /* <tasktimeout> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(26);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+tasktimeout_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+tasktimeout_tag_length); j++)
					if(tasktimeout_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(26);
				}
				else return report_parse_error(26);
				i += tasktimeout_tag_length + 2;
				/* <tasktimeout> */	
					
				/* </tasktimeout> */
				temp_str = "";
				for(; i < xf->file_length; i++)
					if(xf->file[i] == '<') break;
					else
					{
					   temp_str.push_back(xf->file[i]);
					}
				stringstream ss_to(temp_str);
				ss_to >> ut.timeout;
				if(!(ut.timeout >= 0.1 && ut.timeout <= 1000000000))
						return report_validation_error(12);
		
				if(i >= xf->file_length) return report_parse_error(27);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+tasktimeout_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+tasktimeout_tag_length); j++)
						if(tasktimeout_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(27);
				}
				else return report_parse_error(27);
				i += tasktimeout_tag_length + 3;
	
				/* </tasktimeout> */	
				
				
				/* <networklatencytime> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(46);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+networklatencytime_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+networklatencytime_tag_length); j++)
					if(networklatencytime_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(46);
				}
				else return report_parse_error(46);
				i += networklatencytime_tag_length + 2;
				/* <networklatencytime> */	
					
				/* </networklatencytime> */
				temp_str = "";
				for(; i < xf->file_length; i++)
					if(xf->file[i] == '<') break;
					else
					{
					   temp_str.push_back(xf->file[i]);
					}
				stringstream ss_nto(temp_str);
				ss_nto >> ut.network_latency_time;
				if(!(ut.network_latency_time >= 0.1 && ut.network_latency_time <= 1000000000))
						return report_validation_error(25);
		
				if(i >= xf->file_length) return report_parse_error(47);
				
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+networklatencytime_tag_length+1] == '>')
				{
					
					for(j = i+2; j < (i+2+networklatencytime_tag_length); j++)
						if(networklatencytime_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(47);
				}
				else return report_parse_error(47);
				
				i += networklatencytime_tag_length + 3;
	
				/* </networklatencytime> */	
				
				
				/* <tasksourcepath> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(28);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+tasksourcepath_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+tasksourcepath_tag_length); j++)
					if(tasksourcepath_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(28);
				}
				else return report_parse_error(28);
				i += tasksourcepath_tag_length + 2;
				/* <tasksourcepath> */	
					
				/* </tasksourcepath> */
					
				int ts_index = i;
				int ts_tag_set = 1;
				int tot_ts_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				ts_index = i;
				while(ts_tag_set)
				{
					
					if(xf->file[ts_index] == '<' && xf->file[ts_index+ts_tag_length+1] == '>')
					{
						for(j = ts_index+1; j < (ts_index+1+ts_tag_length); j++)
						if(ts_tag[j-(ts_index+1)] != xf->file[j])
						{ 
							ts_tag_set = 0; break; 
						}
						
						if(ts_tag_set == 0) break;
					
						tot_ts_tags += 1;

			        /* <ts> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
				
				if(i >= xf->file_length) return report_parse_error(48);
					//i += 1;
				if(xf->file[i] == '<' && xf->file[i+ts_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+ts_tag_length); j++)
					if(ts_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(48);
				}
				else return report_parse_error(48);
				i += ts_tag_length + 2;
				/* <ts> */	
					
				/* </ts> */
				string temp_dep = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_dep.size() > 40)
					{	//return validate_error(1); 
					}
					temp_dep.push_back(xf->file[i]);
				}
				if(temp_dep !=  "") ut.task_source_path.push_back(temp_dep);
		
				if(i >= xf->file_length) return report_parse_error(49);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+ts_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+ts_tag_length); j++)
						if(ts_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(49);
				}
				else return report_parse_error(49);
				i += ts_tag_length + 3;
	
				/* </ts> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						ts_index = i;
					}
					else break;
				}
					
					
				if(i >= xf->file_length) return report_parse_error(29);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+tasksourcepath_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+tasksourcepath_tag_length); j++)
						if(tasksourcepath_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(29);
				}
				else return report_parse_error(29);
				i += tasksourcepath_tag_length + 3;
	
				/* </tasksourcepath> */	

				/* <taskinputsetpath> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(42);
	
				if(xf->file[i] == '<' && xf->file[i+taskinputsetpath_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+taskinputsetpath_tag_length); j++)
					if(taskinputsetpath_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(42);
				}
				else return report_parse_error(42);
				i += taskinputsetpath_tag_length + 2;
				/* <taskinputsetpath> */	
					
				/* </taskinputsetpath> */
				
				int ti_index = i;
				int ti_tag_set = 1;
				int tot_ti_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				ti_index = i;
				while(ti_tag_set)
				{
					if(xf->file[ti_index] == '<' && xf->file[ti_index+ti_tag_length+1] == '>')
					{
						for(j = ti_index+1; j < (ti_index+1+ti_tag_length); j++)
						if(ti_tag[j-(ti_index+1)] != xf->file[j])
						{ 
							ti_tag_set = 0; break; 
						}
					
						if(ti_tag_set == 0) break;
					tot_ti_tags += 1;

			        /* <ti> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
			
				if(i >= xf->file_length) return report_parse_error(50);
				
				if(xf->file[i] == '<' && xf->file[i+ti_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+ti_tag_length); j++)
					if(ti_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(50);
				}
				else return report_parse_error(50);
				i += ti_tag_length + 2;
				/* <ti> */	
					
				/* </ti> */
				string temp_dep = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_dep.size() > 40)
					{	//return validate_error(1); 
					}
			
					temp_dep.push_back(xf->file[i]);
				}
			
				if(temp_dep !=  "") ut.task_inputset_path.push_back(temp_dep);
		
				if(i >= xf->file_length) return report_parse_error(51);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+ti_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+ti_tag_length); j++)
						if(ti_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(51);
				}
				else return report_parse_error(51);
				i += ti_tag_length + 3;
	
				/* </ti> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						ti_index = i;
					}
					else break;
				}

				if(ut.task_inputset_path.size() == 0)
				   return report_validation_error(15);
		
				if(i >= xf->file_length) return report_parse_error(43);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+taskinputsetpath_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+taskinputsetpath_tag_length); j++)
						if(taskinputsetpath_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(43);
				}
				else return report_parse_error(43);
				i += taskinputsetpath_tag_length + 3;
	
				/* </taskinputsetpath> */	


				/* <taskoutputsetpath> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(44);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+taskoutputsetpath_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+taskoutputsetpath_tag_length); j++)
					if(taskoutputsetpath_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(44);
				}
				else return report_parse_error(44);
				i += taskoutputsetpath_tag_length + 2;
				/* <taskoutputsetpath> */	
					
				int to_index = i;
				int to_tag_set = 1;
				int tot_to_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				to_index = i;
				while(to_tag_set)
				{
					if(xf->file[to_index] == '<' && xf->file[to_index+to_tag_length+1] == '>')
					{
						for(j = to_index+1; j < (ti_index+1+to_tag_length); j++)
						if(to_tag[j-(to_index+1)] != xf->file[j])
						{ 
							to_tag_set = 0; break; 
						}
					
						if(to_tag_set == 0) break;
					tot_to_tags += 1;

			        /* <to> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
			
				if(i >= xf->file_length) return report_parse_error(52);
				
				if(xf->file[i] == '<' && xf->file[i+to_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+to_tag_length); j++)
					if(to_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(52);
				}
				else return report_parse_error(52);
				i += to_tag_length + 2;
				/* <to> */	
					
				/* </to> */
				string temp_dep = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_dep.size() > 40)
					{	//return validate_error(1); 
					}
			
					temp_dep.push_back(xf->file[i]);
				}
			
				if(temp_dep !=  "") ut.task_outputset_path.push_back(temp_dep);
		
				if(i >= xf->file_length) return report_parse_error(53);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+to_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+to_tag_length); j++)
						if(to_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(53);
				}
				else return report_parse_error(53);
				i += to_tag_length + 3;
	
				/* </to> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						to_index = i;
					}
					else break;
				}


				if(ut.task_outputset_path.size() == 0)
				   return report_validation_error(21);
		
				if(i >= xf->file_length) return report_parse_error(45);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+taskoutputsetpath_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+taskoutputsetpath_tag_length); j++)
						if(taskoutputsetpath_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(45);
				}
				else return report_parse_error(45);
				i += taskoutputsetpath_tag_length + 3;
	
				/* </taskoutputsetpath> */	


				/* <taskcompilecommand> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(30);
				
	
				if(xf->file[i] == '<' && xf->file[i+taskcompilecommand_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+taskcompilecommand_tag_length); j++)
					if(taskcompilecommand_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(30);
				}
				else return report_parse_error(30);
				i += taskcompilecommand_tag_length + 2;
				/* <taskcompilecommand> */	
					
				/* </taskcompilecommand> */
				
				
				int tc_index = i;
				int tc_tag_set = 1;
				int tot_tc_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				tc_index = i;
				while(tc_tag_set)
				{
					if(xf->file[tc_index] == '<' && xf->file[tc_index+tc_tag_length+1] == '>')
					{
						for(j = tc_index+1; j < (ti_index+1+tc_tag_length); j++)
						if(tc_tag[j-(tc_index+1)] != xf->file[j])
						{ 
							tc_tag_set = 0; break; 
						}
					
						if(tc_tag_set == 0) break;
					tot_tc_tags += 1;

			        /* <tc> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
			
				if(i >= xf->file_length) return report_parse_error(54);
					//i += 1;
				if(xf->file[i] == '<' && xf->file[i+tc_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+tc_tag_length); j++)
					if(tc_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(54);
				}
				else return report_parse_error(54);
				i += tc_tag_length + 2;
				/* <tc> */	
					
				/* </tc> */
				string temp_dep = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_dep.size() > 40)
					{	//return validate_error(1); 
					}
			
					temp_dep.push_back(xf->file[i]);
				}
				if(temp_dep !=  "") ut.task_compile_command.push_back(temp_dep);
		
				if(i >= xf->file_length) return report_parse_error(55);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+tc_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+tc_tag_length); j++)
						if(tc_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(55);
				}
				else return report_parse_error(55);
				i += tc_tag_length + 3;
	
				/* </tc> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						tc_index = i;
					}
					else break;
				}

	
				if(ut.task_compile_command.size() == 0)
					return report_validation_error(17);
		
				if(i >= xf->file_length) return report_parse_error(31);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+taskcompilecommand_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+taskcompilecommand_tag_length); j++)
						if(taskcompilecommand_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(31);
				}
				else return report_parse_error(31);
				i += taskcompilecommand_tag_length + 3;
	
				/* </taskcompilecommand> */	
				
				/* <taskexecutioncommand> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(32);
	
				if(xf->file[i] == '<' && xf->file[i+taskexecutioncommand_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+taskexecutioncommand_tag_length); j++)
					if(taskexecutioncommand_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(32);
				}
				else return report_parse_error(32);
				i += taskexecutioncommand_tag_length + 2;
				/* <taskexecutioncommand> */	
					
				/* </taskexecutioncommand> */
				ut.task_execution_command = "";
				for(; i < xf->file_length; i++)
					if(xf->file[i] == '<') break;
					else
					{
						if(ut.task_execution_command.size() > 300)
							return report_validation_error(20);
						ut.task_execution_command.push_back(xf->file[i]);
					}
				if(ut.task_execution_command.size() == 0)
					return report_validation_error(19);
		
				if(i >= xf->file_length) return report_parse_error(33);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+taskexecutioncommand_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+taskexecutioncommand_tag_length); j++)
						if(taskexecutioncommand_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(33);
				}
				else return report_parse_error(33);
				i += taskexecutioncommand_tag_length + 3;
	
				/* </taskexecutioncommand> */	
				
				/* </task> */
				for(; i < xf->file_length; i++)
					if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(17);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+task_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+task_tag_length); j++)
						if(task_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(17);
				}
				else return report_parse_error(17);
				i += task_tag_length + 3;
	
				/* </task> */
				pxe->Tasks.push_back(ut);
					
				for(; i < xf->file_length; i++)
					if(xf->file[i] != ' ') break;
				cur_index = i;
		}
		else break;
	}
	
	/* </tasks> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(15);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+tasks_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+tasks_tag_length); j++)
				if(tasks_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(15);
	}
	else return report_parse_error(15);
	i += tasks_tag_length + 3;
	
	/* </tasks> */

        /* <rcp> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(34);
	
	if(xf->file[i] == '<' && xf->file[i+rcp_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+rcp_tag_length); j++)
				if(rcp_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(34);
	}
	else return report_parse_error(34);
	i += rcp_tag_length + 2;
	/* <rcp> */


        /* <rcpsourcepath> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(36);
	
	if(xf->file[i] == '<' && xf->file[i+rcpsourcepath_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+rcpsourcepath_tag_length); j++)
				if(rcpsourcepath_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(36);
	}
	else return report_parse_error(36);
	i += rcpsourcepath_tag_length + 2;
	/* <rcpsourcepath> */
	
	/* </rcpsourcepath> */
	pxe->RCP.rcp_path = "";
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
		else
		{
			if(pxe->RCP.rcp_path.size() > MAXPATHLENGTH)
			{	//return validate_error(1); 
			}
			
			pxe->RCP.rcp_path.push_back(xf->file[i]);
		}

		
	if(i >= xf->file_length) return report_parse_error(37);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+rcpsourcepath_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+rcpsourcepath_tag_length); j++)
				if(rcpsourcepath_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(37);
	}
	else return report_parse_error(37);
	i += rcpsourcepath_tag_length + 3;
	
	/* </rcpsourcepath> */


	
	/* </rcp> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(35);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+rcp_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+rcp_tag_length); j++)
				if(rcp_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(35);
	}
	else return report_parse_error(35);
	i += rcp_tag_length + 3;
	
	/* </rcp> */

	    /* <emp> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(38);
	
	if(xf->file[i] == '<' && xf->file[i+emp_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+emp_tag_length); j++)
				if(emp_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(38);
	}
	else return report_parse_error(38);
	i += emp_tag_length + 2;
	/* <emp> */


        /* <emptimeout> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;

	if(i >= xf->file_length) return report_parse_error(40);

	if(xf->file[i] == '<' && xf->file[i+emptimeout_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+emptimeout_tag_length); j++)
				if(emptimeout_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(40);
	}
	else return report_parse_error(40);
	i += emptimeout_tag_length + 2;
	/* <emptimeout> */
	
	/* </emptimeoutath> */
			
	
	string emp_timeout;
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
		else
		{	
			emp_timeout.push_back(xf->file[i]);
		}
	


	if(i >= xf->file_length) return report_parse_error(41);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+emptimeout_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+emptimeout_tag_length); j++)
				if(emptimeout_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(41);
	}
	else return report_parse_error(41);
	i += emptimeout_tag_length + 3;
	
	stringstream eto(emp_timeout);
	eto >> pxe->EMP.emp_timeout;
		
	/* </emptimeout> */

	 /* <checkpoints> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;

	if(i >= xf->file_length) return report_parse_error(56);

	if(xf->file[i] == '<' && xf->file[i+checkpoints_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+checkpoints_tag_length); j++)
				if(checkpoints_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(56);
	}
	else return report_parse_error(56);
	i += checkpoints_tag_length + 2;
	/* <checkpoints> */


		
				int cp_index = i;
				int cp_tag_set = 1;
				int tot_cp_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				cp_index = i;
				while(cp_tag_set)
				{
					if(xf->file[cp_index] == '<' && xf->file[cp_index+cp_tag_length+1] == '>')
					{
						for(j = cp_index+1; j < (cp_index+1+cp_tag_length); j++)
						if(cp_tag[j-(cp_index+1)] != xf->file[j])
						{ 
							cp_tag_set = 0; break; 
						}
					
						if(cp_tag_set == 0) break;
					
					tot_cp_tags += 1;

			          /* <cp> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(58);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+cp_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+cp_tag_length); j++)
					if(cp_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(58);
				}
				else return report_parse_error(58);
				i += cp_tag_length + 2;
				/* <cp> */	
					
				/* </cp> */
				string temp_cp = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_cp.size() > 40)
					{	//return validate_error(1); 
					}
			
					temp_cp.push_back(xf->file[i]);
				}
				if(temp_cp !=  "") pxe->EMP.checkpoints.push_back(temp_cp);
		
				if(i >= xf->file_length) return report_parse_error(59);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+cp_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+cp_tag_length); j++)
						if(cp_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(59);
				}
				else return report_parse_error(59);
				i += cp_tag_length + 3;
	
				/* </cp> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						cp_index = i;
					}
					else break;
				}
					

	/* </checkpoints> */	
	
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
	
	if(i >= xf->file_length) return report_parse_error(57);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+checkpoints_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+checkpoints_tag_length); j++)
				if(checkpoints_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(57);
	}
	else return report_parse_error(57);
	i += checkpoints_tag_length + 3;
	
	/* </checkpoints> */

	/* <executioncommands> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;

	if(i >= xf->file_length) return report_parse_error(60);

	if(xf->file[i] == '<' && xf->file[i+executioncommands_tag_length+1] == '>')
	{
			for(j = i+1; j < (i+1+executioncommands_tag_length); j++)
				if(executioncommands_tag[j-(i+1)] != xf->file[j])
					return report_parse_error(60);
	}
	else return report_parse_error(60);
	i += executioncommands_tag_length + 2;
	/* <executioncommands> */
	

				int ec_index = i;
				int ec_tag_set = 1;
				int tot_ec_tags = 0;

				for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;

				ec_index = i;
				while(ec_tag_set)
				{
					if(xf->file[ec_index] == '<' && xf->file[ec_index+ec_tag_length+1] == '>')
					{
						for(j = ec_index+1; j < (ec_index+1+ec_tag_length); j++)
						if(ec_tag[j-(ec_index+1)] != xf->file[j])
						{ 
							ec_tag_set = 0; break; 
						}
					
						if(ec_tag_set == 0) break;
					
					tot_ec_tags += 1;

			          /* <ec> */
			  	for(; i < xf->file_length; i++)
						if(xf->file[i] != ' ') break;
		
				if(i >= xf->file_length) return report_parse_error(62);
					//i += 1;
	
				if(xf->file[i] == '<' && xf->file[i+ec_tag_length+1] == '>')
				{
					for(j = i+1; j < (i+1+ec_tag_length); j++)
					if(ec_tag[j-(i+1)] != xf->file[j])
						return report_parse_error(62);
				}
				else return report_parse_error(62);
				i += ec_tag_length + 2;
				/* <ec> */	
					
				/* </ec> */
				string temp_ec = "";
				for(; i < xf->file_length; i++)
				if(xf->file[i] == '<') break;
				else
				{
					if(temp_ec.size() > 40)
					{	//return validate_error(1); 
					}
			
					temp_ec.push_back(xf->file[i]);
				}
				if(temp_ec !=  "") pxe->EMP.emp_execution_commands.push_back(temp_ec);
		
				if(i >= xf->file_length) return report_parse_error(63);
	
				if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+ec_tag_length+1] == '>')
				{
					for(j = i+2; j < (i+2+ec_tag_length); j++)
						if(ec_tag[j-(i+2)] != xf->file[j])
							return report_parse_error(63);
				}
				else return report_parse_error(63);
				i += ec_tag_length + 3;
	
				/* </ec> */
						for(; i < xf->file_length; i++)
							if(xf->file[i] != ' ') break;


						ec_index = i;
					}
					else break;
				}
					


	/* </executioncommands> */	
	
	for(; i < xf->file_length; i++)
		if(xf->file[i] == '<') break;
	
	if(i >= xf->file_length) return report_parse_error(61);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+executioncommands_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+executioncommands_tag_length); j++)
				if(executioncommands_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(61);
	}
	else return report_parse_error(61);
	i += executioncommands_tag_length + 3;
	
	/* </executioncommands> */

	
	/* </emp> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(39);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+emp_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+rcp_tag_length); j++)
				if(emp_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(39);
	}
	else return report_parse_error(39);
	i += emp_tag_length + 3;
	
	/* </emp> */

	
	/* </problem> */
	for(; i < xf->file_length; i++)
		if(xf->file[i] != ' ') break;
		
	if(i >= xf->file_length) return report_parse_error(2);
	
	if(xf->file[i] == '<' && xf->file[i+1] == '/' && xf->file[i+1+problem_tag_length+1] == '>')
	{
		for(j = i+2; j < (i+2+problem_tag_length); j++)
				if(problem_tag[j-(i+2)] != xf->file[j])
					return report_parse_error(2);
	}
	else return report_parse_error(2);
	i += problem_tag_length + 3;
	/* </problem> */
	
	
	for(; xf->file[i] != EOF && xf->file[i] != 0 &&  xf->file_length; i++)
		if(xf->file[i] != ' ') { return report_parse_error(3); }
	
	return validate_parsed_data(pxe);
}

void show_file(XMLFile *xf)
{
	int i;
	for(i = 0; i < (xf->file_length); i++)
		putc(xf->file[i],stdout);
}


int start_to_parse(XMLFile *xf, ParsedXMLElements *pxe, string pss_file)
{
	strcpy(xf->file_address, (char *)pss_file.c_str());
	get_file(xf);
	return parse(xf,pxe);
}
