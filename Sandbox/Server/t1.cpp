/* This is an example task program.
 * This program along with the input set and
 * other files like t1.tdd, etc.
 * would be send to the client, which will
 * then compile and execute it and send the
 * results back */

#include<iostream>
#include<cstdio>
#include<algorithm>
#include<vector>
#include<algorithm>
#define SZ 1000000
using namespace std;
vector<int> arr;
int n;
 
int main()
{
	freopen("t1_inp.inp", "r", stdin);
	freopen("t1_out.out", "w", stdout);
	int temp;
	scanf("%d",&n);
	
	for(int i = 0; i < n; i++)
	{
		scanf("%d",&temp);
		arr.push_back(temp);
	}

	sort(arr.begin(),arr.end());
	
	for(int i = 0; i < n; i++)
	{
		printf("%d\n", arr[i]);
	}
	return 0;	
}
