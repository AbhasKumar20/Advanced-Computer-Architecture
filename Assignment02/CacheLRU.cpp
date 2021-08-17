#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define S 2048
#define W 16


u_int64_t T_S;
u_int64_t C[S][W];
unsigned long long int T_Stamp[S][W];


int Set_W(int cSet, int cTag);


int main(int argc, char *argv[])
{
    char i_Name[40], o_Name[40];
    u_int64_t addr, cacheTag, cacheSet, cacheSetWay;
    int tid;
    
    FILE *fp , * trace;
    
    
    u_int64_t H_Count = 0, M_Count = 0;

    
    sprintf(i_Name, "%s", argv[1]);
    fp = fopen(i_Name, "r");

    
    //sprintf(o_Name, "%s_miss.out", argv[1]);
    trace = fopen("misstrace.out", "w");

   
    if (fp == NULL)
    {
        printf("Error!");
        exit(1);
    }

    while (!feof(fp))
    {
        
        fscanf(fp, "%d  %lx", &tid, &addr); //taking 4 field as input from trace.
        
        cacheSet = (addr >> 6) & 0x7FF;
        cacheTag = (addr >> 17) & 0x7FFFFFFF;

        
        cacheSetWay = Set_W(cacheSet, cacheTag);
        
        
        if (cacheSetWay != -1)
        {
            
            H_Count++;
            T_S++;
            T_Stamp[cacheSet][cacheSetWay] = T_S;
        }
        else
        {
            
            int cacheSetWay;
            M_Count++;
            fprintf(trace,"%d  %lx \n",tid, addr);
            
   
            
            int i, min = T_Stamp[cacheSet][0], caches_Way = 0;
            for (i = 1; i < W; i++)
            {
             
                if (T_Stamp[cacheSet][i] <= min)
                {
                     min = T_Stamp[cacheSet][i];
                     caches_Way = i;
                }
            }
            cacheSetWay=caches_Way;
            
            
            C[cacheSet][cacheSetWay] = cacheTag;
            
            T_S++;
            T_Stamp[cacheSet][cacheSetWay] = T_S;   
            
        }
    }

   
    printf("Hits:%ld  Misses:%ld\n", H_Count, M_Count);
    
    return 0;
}    
    
int Set_W(int cSet, int cTag)
{
    
    for (int i = 0; i < W; i++)
    {
        if ((C[cSet][i] == cTag) && (T_Stamp[cSet][i] != 0))
            return i;
    }
    return -1;
}   
    
   
