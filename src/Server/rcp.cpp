/* RCP for Distributed Sort */

#include<iostream>
#include<cstdlib>
#include<string>
#include<vector>
#include<cstring>
#include<fstream>

using namespace std;

void unzip(vector<string> tasks)
{
	for(int i = 0; i < tasks.size(); i++)
			{
				system( (char *) ( ( "gunzip -f " + tasks[i] + "_op.tar.gz" ).c_str() ));
				system( (char *) ( ( "tar xf " + tasks[i] + "_op.tar" ).c_str()));
			}
}

void process(vector<string> tasks)
{
	ifstream ops[1000];
	
	int i;
	for(i = 0; i < tasks.size(); i++)
	{
		ops[i].open((char *)(tasks[i] + "_out.out").c_str());
		int p; ops[i] >> p;
	}
	ofstream fop;
	fop.open("rcpoutput.out");
	long long mx = -1, mxi;
	
	while(1)
	{
		for(i = 0; i < tasks.size(); i++)
		{
			long long temp;
			//int g = tellg(ops[i]);
			if(ops[i] >> temp)
			{
				//cout << i << " " << temp << endl;
				if(temp > mx) { mx = temp; mxi = i; }
				ops[i].seekg(-1,ios::cur);
			}	
			else
			{
				continue;
			}
		}
		if(mx == -1) break;
		else { fop << mx << endl; 
		
		mx = -1; ops[mxi].seekg(1,ios::cur); }
	}

	for(i = 0; i < tasks.size(); i++)
		ops[i].close();
	fop.close();
}

int main()
{
	vector<string> tasks;
	tasks.push_back("t1"); 	
	tasks.push_back("t2"); 
	tasks.push_back("t3"); 
	tasks.push_back("t4"); 
	tasks.push_back("t5"); 
	tasks.push_back("t6"); 
	tasks.push_back("t7"); 
	tasks.push_back("t8");
	tasks.push_back("t9");  
	tasks.push_back("t10"); 
	tasks.push_back("t11"); 	
	tasks.push_back("t12"); 
	tasks.push_back("t13"); 
	tasks.push_back("t14"); 
	tasks.push_back("t15"); 
	tasks.push_back("t16"); 
	tasks.push_back("t17"); 
	tasks.push_back("t18");
	tasks.push_back("t19");  
	tasks.push_back("t20"); 
	unzip(tasks);
	
	process(tasks);
	
	return 0;
}
