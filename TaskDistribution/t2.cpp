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
