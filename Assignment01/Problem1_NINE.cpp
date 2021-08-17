#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstring>
using namespace std;


#define SHIFT_RIGHT(x,k) (x>>k)
#define SET_NUM(x,n) (((1<<n)-1) & x)


struct Cache
{
	int present;
	unsigned long long tag,time;
};
void initialiseCache(vector< vector<Cache> > &C,int sets,int lines )
{														// Inisitialise all fields of L2 & L3 Cache
	for (int x = 0; x<sets;x++)
	{
		for(int y=0;y<lines;y++)
		{
			C[x][y].present=0;
			C[x][y].time=0;
			C[x][y].tag=0;

		}
	}

}

bool SearchL2(vector< vector<Cache> > &L2 ,int L2_SET,unsigned long long L2_TAG,unsigned long long *priority){
															// Search in L2 Cache

	for(int line =0; line<8;++line)
	{	
		if((L2[L2_SET][line].present==1) && (L2[L2_SET][line].tag==L2_TAG))
		{
			L2[L2_SET][line].time=++(*priority);
			return true;
		}
	}
	return false;
}

void evictL2(vector< vector<Cache> > &L2 , int L2_SET,unsigned long long L2_TAG, unsigned long long min, unsigned long long* priority ){
	
	int index=0;											// Replace in L2 Cache
	for(int line=1; line<8; ++line)
	
		if(L2[L2_SET][line].time < min){
		{
			min = L2[L2_SET][line].time;		
			index = line;
		}
	}
	
	L2[L2_SET][index].tag = L2_TAG;
	L2[L2_SET][index].time = ++(*priority);

}

void evictL3(vector< vector<Cache> > &L3,int L3_SET,unsigned long long L3_TAG, unsigned long long min, unsigned long long* priority )
{													// Replace in L3 Cache 
	int index=0;
	for(int line=1; line<16; ++line)
	{
		if(L3[L3_SET][line].time < min)
		{
			min = L3[L3_SET][line].time;	
			index = line;
		}
	
	}

	L3[L3_SET][index].tag = L3_TAG;
	L3[L3_SET][index].time = ++(*priority);
}



int main(int argc, char** argv)
{
	string applications[6] = { "h264ref.log_l1misstrace", "hmmer.log_l1misstrace", "bzip2.log_l1misstrace", 
							"sphinx3.log_l1misstrace","gcc.log_l1misstrace","gromacs.log_l1misstrace"};
	int numtraces[6]={1,1,2,2,2,1};
	cout<< "-------------------------------------------------------------------------" << "\n";


	for(int i=0;i<6;++i)
	{
		clock_t tStart = clock();
	 	cout << "Tracing: " << applications[i] <<"\n";
		vector<vector<Cache>> L2(1024, vector<Cache>(8));
		vector<vector<Cache>> L3(2048, vector<Cache>(16));

		initialiseCache(L2,1024,8);
		initialiseCache(L3,2048,16);
		
		int L2_hits=0,L2_misses=0,L3_hits=0,L3_misses=0,L2_SET=0,L3_SET;
		bool L3_HIT=false,empty=false;
	
		FILE * fp;
		char iord,type;
		unsigned long long addr,L2_TAG,L3_TAG,priority=0,min,L3_ReTAG, L1_misses=0;
		unsigned pc;
		for (int k=0; k<numtraces[i]; k++)
		{
			
			fp = fopen((applications[i] + "_" +to_string(k)).c_str(), "rb");
			assert(fp != NULL);

		  	while (!feof(fp))
		  	{
				fread(&iord, sizeof(char), 1, fp);
				fread(&type, sizeof(char), 1, fp);
				fread(&addr, sizeof(unsigned long long), 1, fp);
				fread(&pc, sizeof(unsigned), 1, fp);


				if(type=='0')
					continue;

				L1_misses++;

				addr=SHIFT_RIGHT(addr,6);    //remove block offset from addr
				L2_SET=SET_NUM(addr,10);     //Extract Set number for L2 Cache from addr 
				L2_TAG=SHIFT_RIGHT(addr,10); //Extract Tag for L2 Cache from addr
			
				
				L3_HIT=false,empty=false;

				if(SearchL2(L2,L2_SET,L2_TAG,&priority)) //Search in L2 cache
					L2_hits++; 

		     	else
		     	{
		     		
					L2_misses++;                        // Miss in L2

					L3_SET=SET_NUM(addr,11);           //Extract Set number for L3 Cache from addr 
					L3_TAG=SHIFT_RIGHT(addr,11);       //Extract Tag number for L3 Cache from addr 
					
					
					for(int line=0;line<16;++line)
					{
						if((L3[L3_SET][line].present==1) && (L3[L3_SET][line].tag==L3_TAG)) //Search in L3 Cache
						{
							L3_hits++;
							L3_HIT=true;
							L3[L3_SET][line].time=++priority;


							for (int i = 0; i < 8; ++i)
							{
								if(L2[L2_SET][i].present==0)           // IF present in L3 Cache ,Try to Place in L2 Cache
								{
								empty=true;
								L2[L2_SET][i].present=1;
								L2[L2_SET][i].tag=L2_TAG;
								L2[L2_SET][i].time=++priority;
								break;

								}
							}
							
							if(empty==false)
								evictL2(L2,L2_SET,L2_TAG,L2[L2_SET][0].time,&priority);  // Replacement in L2 Cache

							break;

						}

					}

					
		     		if(L3_HIT==false)
		     		{
		     			L3_misses++;                       // Miss in L2 and also in L3 Cache

		     			empty=false;
		     			for (int i = 0; i < 16; ++i)
		     			{
		     				if(L3[L3_SET][i].present==0)     //Try to place in L3 Cache
		     				{
		     					empty=true;
		     					L3[L3_SET][i].present=1;
		     					L3[L3_SET][i].tag=L3_TAG;
		     					L3[L3_SET][i].time=++priority;
		     					break;

		     				}
		     			}
		     			
		     			if(empty==false)                                 //If can't be placed in L3 cache
		     				evictL3(L3,L3_SET,L3_TAG,L3[L3_SET][0].time,&priority); //Replace in L3 also evict in L2 
							
		     			
		     			empty=false;
		     			for (int line = 0; line < 8; ++line)
		     			{
		     				if(L2[L2_SET][line].present==0)			//Try to Place in L2 Cache
		     				{
		     					empty=true;
		     					L2[L2_SET][line].present=1;
		     					L2[L2_SET][line].tag=L2_TAG;
		     					L2[L2_SET][line].time=++priority;
		     					break;

		     				}
		     			}
		     			
	     				if(empty==false)                                            //If can't be placed in L2 cache
	     					evictL2(L2,L2_SET,L2_TAG,L2[L2_SET][0].time,&priority);  //Replace in L2 Cache
						
		     		
		     		}
			
		     	} 	
			
		     
		  	}
		  	fclose(fp);
			//printf("Done reading file %d!\n", k);
		  
		}

		cout << "L1 misses:" << L1_misses <<"\n";
		cout << "L2 misses:" << L2_misses <<"\n";
		cout << "L3 misses:" << L3_misses << "\n";
		cout << "L2 hits:" << L2_hits <<"\n";
		cout << "L3 hits:" << L3_hits <<"\n";
		cout <<"Execution time:" <<((double)(clock() - tStart))/CLOCKS_PER_SEC <<" seconds"<<"\n";
		cout <<"---------------------------------------------------------------------"<<"\n";
	}

 return 0;
}
