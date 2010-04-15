#include<iostream>
#include<vector>
#include<cstdlib>
#include<fstream>

int create_task_archives(XMLFile *xf, ParsedXMLElements *pxe)
{
	//TODO
	/* Copying the files into the current directory */

	for(int i = 0; i < (pxe->Tasks.size()); i++)
	{
		string cur_task_id = pxe->Tasks[i].task_id;
		ofstream task_desc_file;
		task_desc_file.open( ((cur_task_id)+".tdd").c_str());
		task_desc_file << cur_task_id << endl;
		
		task_desc_file << (pxe->Tasks[i].task_compile_command.size()) << endl;
		for(int j = 0; j < (pxe->Tasks[i].task_compile_command.size()); j++)
			task_desc_file << (pxe->Tasks[i].task_compile_command[j]) << endl;
		
		task_desc_file << (pxe->Tasks[i].task_execution_command) << endl;
		
		task_desc_file << (pxe->Tasks[i].task_inputset_path.size()) << endl;
		for(int j = 0; j < (pxe->Tasks[i].task_inputset_path.size()); j++)
			task_desc_file << (pxe->Tasks[i].task_inputset_path[j]) << endl;
				
		task_desc_file << (pxe->Tasks[i].task_outputset_path.size()) << endl;
		for(int j = 0; j < (pxe->Tasks[i].task_outputset_path.size()); j++)
			task_desc_file << (pxe->Tasks[i].task_outputset_path[j]) << endl;
		
		task_desc_file << (pxe->Tasks[i].priority) << endl;
		task_desc_file << (pxe->Tasks[i].timeout) << endl;

		task_desc_file.close();
			
		
		string command = "tar cf " + cur_task_id + ".tar " + cur_task_id + ".tdd";
		system(command.c_str());
		
		for(int j = 0; j < (pxe->Tasks[i].task_source_path.size()); j++)
		{
			command = ("tar rf " + cur_task_id + ".tar " + pxe->Tasks[i].task_source_path[j]);
			system(command.c_str());
		}
		
		for(int j = 0; j < (pxe->Tasks[i].task_inputset_path.size()); j++)
		{
			command = "tar rf " + cur_task_id + ".tar " + pxe->Tasks[i].task_inputset_path[j];
			system(command.c_str());
		}
		
		command = ("gzip -f " + cur_task_id + ".tar");
		system(command.c_str());
	}

	return 0;
}
