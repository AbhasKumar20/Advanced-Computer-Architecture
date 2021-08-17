#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iterator>
#include <map>
#include <iomanip> 

using namespace std;

int main(int argc, char const *argv[])
{
	map< unsigned long long , unsigned long> VPN_AccessNum;
	map<unsigned int, long double> DistFrequency;

	FILE *fp;
    char input_name[40];

    unsigned long long addr;
    unsigned int AccessNum = 0, N=0;

    int threadid, AccessDist;

    sprintf(input_name, "%s", argv[1]);
    fp = fopen(input_name, "r");

    ofstream outFile("CDF.out");

    map<unsigned long long, unsigned long>::iterator itr1;
    map<unsigned int, long double>::iterator itr2,it;


    while (!feof(fp))
    {
        
        fscanf(fp, "%d  %llx", &threadid, &addr);

      
        addr = addr >>6;


        AccessNum++;

        itr1 = VPN_AccessNum.find(addr);

        if(itr1==VPN_AccessNum.end())
        	VPN_AccessNum.insert(pair<unsigned long long, unsigned int>(addr, AccessNum));
        else
        {
        	
        	N++;
        	AccessDist = AccessNum - itr1->second;

        	itr2= DistFrequency.find(AccessDist);
        	if(itr2==DistFrequency.end())
        		DistFrequency.insert(pair<unsigned int, long double>(AccessDist,1));
        	else
        		itr2->second=itr2->second+1;


        	itr1->second=AccessNum;
        }


    }
    

    it= DistFrequency.begin();
    itr2=DistFrequency.begin();
    itr2++;



    for (;itr2!=DistFrequency.end(); it++,itr2++)
    	itr2->second=itr2->second + it->second;

    

    for ( itr2 = DistFrequency.begin(); itr2!= DistFrequency.end(); ++itr2)
    {
    	itr2->second=itr2->second/N;
    	outFile << '\t' << itr2->first 
             << '\t' << std::setprecision(10) << itr2->second << '\n';
    }


	
	return 0;
}