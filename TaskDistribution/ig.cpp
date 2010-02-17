#include<iostream>
#include<cstdlib>

using namespace std;

int main()
{
	srand(time(0));
	int n = rand() % 10000 + 1;
	cout << n << endl;
	for(int i = 0; i < n; i++)
	{
		cout << rand() % 1000000000 + 1 << endl;
	}
	return 0;
}
