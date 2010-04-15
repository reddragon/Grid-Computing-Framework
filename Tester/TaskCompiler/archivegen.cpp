#include<iostream>
#include<vector>
#include<cstdlib>
#include<fstream>
#include "parser.h"
int main()
{
	XMLFile *xf = new XMLFile;
	if(xf == NULL) return 1;
	
	ParsedXMLElements *pxe = new ParsedXMLElements;

	strcpy(xf->file_address, "MergeSort.pss");
	get_file(xf);
	parse(xf,pxe);
	//TODO
	/* Copying the files into the current directory */

	for(int i = 0; i < (pxe->Tasks.size()); i++)
	{
		cout << pxe->Tasks[i].task_id << endl;
		string cur_task_id = pxe->Tasks[i].task_id;
		ofstream task_desc_file;
		task_desc_file.open( ((cur_task_id)+".tdd").c_str());
		task_desc_file << cur_task_id << endl;
		task_desc_file << (pxe->Tasks[i].task_compile_command) << endl;
		task_desc_file << (pxe->Tasks[i].task_execution_command) << endl;
		task_desc_file << (pxe->Tasks[i].task_inputset_path) << endl;
		task_desc_file << (pxe->Tasks[i].task_outputset_path) << endl;
		task_desc_file << (pxe->Tasks[i].priority) << endl;
		task_desc_file << (pxe->Tasks[i].timeout) << endl;

		task_desc_file.close();
			
		string command = ("tar cf " + cur_task_id + ".tar " + pxe->Tasks[i].task_source_path);
		system(command.c_str());
		command = "tar rf " + cur_task_id + ".tar " + cur_task_id + "_inp.inp";
		system(command.c_str());
		command = "tar rf " + cur_task_id + ".tar " + cur_task_id + ".tdd";
		system(command.c_str());
		command = ("gzip -f " + cur_task_id + ".tar");
		system(command.c_str());

	}

	
	delete xf; delete pxe;

	return 0;
}
