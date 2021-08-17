#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <algorithm>
#include <cassert>
#include <cstring>
using namespace std;


#define PB push_back
#define SHIFT_RIGHT(x,k) (x>>k)
#define SET_NUM(x,n) (((1<<n)-1) & x)


struct Cache
{
	int present;
	unsigned long long tag,time;
};

void initialiseL2Cache(vector< vector<Cache> > &L2,int sets,int lines )
{														// Inisitialise all fields of L2 
	for (int x = 0; x<sets;x++)
	{
		for(int y=0;y<lines;y++)
		{
			L2[x][y].present=0;
			L2[x][y].time=0;
			L2[x][y].tag=0;

		}
	}

}
void initialiseL3Cache(vector<Cache> &L3)
{														// Inisitialise all required fields of L3 Cache
	for (int line = 0; line < L3.size(); ++line)
	{
		L3[line].present=0;
		L3[line].tag=0;
	}

}


bool SearchL2(vector< vector<Cache> > &L2 ,int L2_SET,unsigned long long L2_TAG,unsigned long long *priority)
{
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

void evictL2(vector< vector<Cache> > &L2 , int L2_SET,unsigned long long L2_TAG, unsigned long long min, unsigned long long* priority )
{
	
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
	L2[L2_SET][index].present=1;

}

void evictL3(vector<Cache> &L3 , vector< vector<Cache> > &L2, vector<list<unsigned long> > &Access , unsigned long long L3_TAG)
{	

	int evict_line=0, last_reference=0;									// Replace in L3 Cache 
	for(int line=0; line< L3.size(); line++)
	{
		if(Access[L3[line].tag].size()==0)
		{
			//If a block is never accessed in the future, evict that block.
			evict_line = line;
			break;
		}
		else
		{
			if(Access[L3[line].tag].front() > last_reference)
			{
				last_reference = Access[L3[line].tag].front();
				evict_line = line;
			}
		}
	}

	unsigned long long L3_ReTAG = L3[evict_line].tag;				

	unsigned long long L2_SET = SET_NUM(L3_ReTAG,10); 		//Extract L2 Set number from L3 tag and Set number
	unsigned long long L2_TAG = SHIFT_RIGHT(L3_ReTAG,10);    //Extract L2 Tag from L3 Tag and Set number


	L3[evict_line].tag = L3_TAG;										// Place in L3 Cache
	L3[evict_line].present = 1;

	for(int line=0; line<8; ++line)								// Remove from L2 Cache if present
	{
		if ((L2[L2_SET][line].present ==1) && (L2[L2_SET][line].tag == L2_TAG ))
		{
			L2[L2_SET][line].present = 0;
			break;
		}
	}	
}

int main(int argc, char** argv)
{
	string applications[6] = { "h264ref.log_l1misstrace", "hmmer.log_l1misstrace", "gromacs.log_l1misstrace","bzip2.log_l1misstrace" 
									,"gcc.log_l1misstrace","sphinx3.log_l1misstrace"};
	int numtraces[6]={1,1,1,2,2,2};
	string expected_time[6]={"7 minutes","9 minutes","5 minutes","27 minutes","24 minutes","97 minutes"};
	cout<< "-------------------------------------------------------------------------" << "\n";

	for (int i = 0; i < 6; ++i)
	{
		clock_t tStart = clock();
	 	cout << "Tracing: " << applications[i] <<"\n";
	 	cout << "expected Execution time:" << expected_time[i]<<"\n";


		unsigned pc;
		unsigned long long addr;
		char iord, type;
		FILE *fp;
		int refrence = 0;
		
		vector<list<unsigned long>> Access(201392128, list<unsigned long>(0));

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
				Access[SHIFT_RIGHT(addr,6)].PB(++refrence);
					
			}

			fclose(fp);
		}


		vector< vector<Cache>> L2(1024, vector<Cache>(8));
		vector<Cache> L3(32768);
		vector<bool> cold(201392128,false);

		initialiseL2Cache(L2,1024,8);
		initialiseL3Cache(L3);
		
		int L2_hits=0,L2_misses=0,L3_hits=0,L3_misses=0,L2_SET=0;
		bool L3_HIT=false,empty=false;
		unsigned long long L2_TAG,L3_TAG,priority=0,L1_misses=0,L3_capacityMiss=0;
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

					addr=SHIFT_RIGHT(addr,6);
					if(Access[addr].size()>0)                           //remove block offset from addr
						Access[addr].pop_front();
					cold[addr]=true;
					L3_TAG=addr;
					L2_SET=SET_NUM(addr,10);                         //Extract Set number for L2 Cache from addr 
					L2_TAG=SHIFT_RIGHT(addr,10);                    //Extract Tag for L2 Cache from addr
				
					
					L3_HIT=false,empty=false;

					if(SearchL2(L2,L2_SET,L2_TAG,&priority))               //Search in L2 cache
						L2_hits++; 

			     	else
			     	{
			     		
						L2_misses++;                                     // Miss in L2
						
						for(int line=0;line<L3.size();++line)
						{
							if((L3[line].present==1) && (L3[line].tag==L3_TAG))          //Search in L3 Cache
							{
								L3_hits++;
								L3_HIT=true;

								for (int i = 0; i < 8; ++i)
								{
									if(L2[L2_SET][i].present==0)         // IF present in L3 Cache ,Try to Place in L2 Cache
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
			     			for (int line = 0; line < L3.size(); ++line)
			     			{
			     				if(L3[line].present==0)     //Try to place in L3 Cache in L3_SET
			     				{
			     					empty=true;
			     					L3[line].present=1;
			     					L3[line].tag=L3_TAG;
			     					break;

			     				}
			     			}
			     			
			     			if(empty==false)						     // When can't be placed in L3 Cache
			     			{
		                        L3_capacityMiss++;                     
		                        											//If can't be placed in L3_SET
			     				evictL3(L3,L2,Access,L3_TAG);           			//Replace in L3 also evict in L2 

							}
			     			
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
		  
		}
		cout << "L3_coldMisses:" << count(cold.begin(),cold.end(),true) <<"\n";
		cout << "L3_capacityMisses:" << L3_capacityMiss << "\n";
		cout << "Execution time:" <<((double)(clock() - tStart))/CLOCKS_PER_SEC <<" seconds"<<"\n";
		cout <<"---------------------------------------------------------------------"<<"\n";
	}

 	return 0;
}