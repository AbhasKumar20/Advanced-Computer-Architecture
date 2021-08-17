#include <iostream>
#include <stdio.h>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

int main(int argc, char const *argv[])
{
	map< unsigned long long , vector<bool> > VPN_AccessNum;
	vector<bool> Threads;
	unsigned int Profile[8]={};

	FILE *fp;
    char input_name[40];

    unsigned long long addr;

    int threadid,thread_count=0,sum=0;

    sprintf(input_name, "%s", argv[1]);
    fp = fopen(input_name, "r");

    map< unsigned long long , vector<bool> > ::iterator it;

    while (!feof(fp))
    {
        
        fscanf(fp, "%d  %llx", &threadid, &addr);

        addr = addr >>6;
        Threads.resize(8,false); 

        it = VPN_AccessNum.find(addr);

        if(it ==VPN_AccessNum.end())
        {
        	Threads[threadid]=true;
        	VPN_AccessNum[addr]=Threads;
        }
        else
        {	
  			Threads=it->second;
  			Threads[threadid]=true;
  			it->second=Threads;
        }

    }

 	
	for ( it = VPN_AccessNum.begin(); it != VPN_AccessNum.end(); ++it)
	{
		thread_count=count(it->second.begin(),it->second.end(),true);
		Profile[thread_count-1]+=1;
	}

	cout << "#threads  " << "#Blocks Sharing" << "\n" ;
	for (int i = 0; i < 8; ++i)
	{
		cout << i+1<< "\t\t" <<Profile[i]<<"\n";
		sum+=Profile[i];
	}

	cout << "Sum:" << sum <<"\n";

	return 0;

}