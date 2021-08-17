/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2018 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */


#include<iostream>
#include "pin.H"

using namespace std;


FILE * trace;
PIN_LOCK pinLock;

INT32 Usage();
VOID Fini(INT32 code, VOID *v);
VOID Instruction(INS ins, VOID *v);
VOID ReadLock(VOID * ip, VOID * addr, UINT32 size, THREADID threadid);
VOID WriteLock(VOID * ip, VOID * addr, UINT32 size, THREADID threadid);
VOID ReadBytes(unsigned long long opAddr, UINT32 opSize, THREADID threadid);
VOID WiteBytes(unsigned long long opAddr, UINT32 opSize, THREADID threadid);



int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen("trace.out", "w");
    
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();
    
    return 0;
}


INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}


VOID Instruction(INS ins, VOID *v)
{

    UINT32 memOperands = INS_MemoryOperandCount(ins);

    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        UINT32 opSize = INS_MemoryOperandSize(ins, memOp);

        if (INS_MemoryOperandIsRead(ins, memOp))
        {   
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)ReadLock,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, opSize,
                IARG_THREAD_ID,
                IARG_END);
        }
        
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)WriteLock,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, opSize,
                IARG_THREAD_ID,
                IARG_END);
        }
    }

}

VOID ReadLock(VOID * ip, VOID * addr, UINT32 size, THREADID threadid)
{
    PIN_GetLock(&pinLock, threadid+1);
    unsigned long long addr1 = (unsigned long long)addr;
    ReadBytes(addr1,size,threadid);
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}
VOID WriteLock(VOID * ip, VOID * addr, UINT32 size, THREADID threadid)
{
    PIN_GetLock(&pinLock, threadid+1);
    unsigned long long addr1 = (unsigned long long)addr;
    WiteBytes(addr1,size,threadid);
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}


VOID ReadBytes(unsigned long long opAddr, UINT32 opSize, THREADID threadid)
{
  
    while (opSize > 0)
    {

        while ((64-(opAddr%64))>=8 && opSize >= 8)
        {
            fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
	    
            opAddr += 8;
            opSize -= 8;
           
        }
        
        while ( (64-(opAddr%64))>=4 && opSize >= 4)
        {
            fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
            opAddr += 4;
            opSize -= 4;
           
        }

        while ((64-(opAddr%64))>=2 && opSize >= 2)
        {
            fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
            opAddr += 2;
            opSize -= 2;
           
        }

        while ((64-(opAddr%64))>=1 &&  opSize >= 1)
        {
           fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
            opAddr += 1;
            opSize -= 1;
        }
	
    }

}

VOID WiteBytes(unsigned long long opAddr, UINT32 opSize, THREADID threadid)
{

    while (opSize > 0)
    {
  
        while ((64-(opAddr%64))>=8 && opSize >= 8)
        {
            fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
	    
            opAddr += 8;
            opSize -= 8;
           
        }
        
        while ((64-(opAddr%64))>=4 && opSize >= 4)
        {
            fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
            opAddr += 4;
            opSize -= 4;
           
        }

        while ((64-(opAddr%64))>=2 && opSize >= 2)
        {
            fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
            opAddr += 2;
            opSize -= 2;
           
        }

        while ((64-(opAddr%64))>=1 &&  opSize >= 1)
        {
           fprintf(trace,"%d 0x%llx\n",threadid, opAddr);
            opAddr += 1;
            opSize -= 1;
        }
	
    }

}  

VOID Fini(INT32 code, VOID *v)
{
    fclose(trace);
	
}



