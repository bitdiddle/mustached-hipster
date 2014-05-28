#include <stdio.h>
#include "pin.H"

FILE * trace;

#define Print2File

// This function is called before every instruction is executed
// and prints the IP
VOID printip(VOID *ip, VOID *target, BOOL taken)
{
    #ifdef Print2File
    fprintf(trace, "%p %p %s\n", ip, target, (taken? "Y": "N"));
    #else
    printf("%p %p %s\n", ip, target, (taken? "Y": "N"));
    #endif
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    // Insert a call to printip before every instruction, and pass it the IP
    if (INS_IsBranch(ins))
    {
        // IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, and IARG_BRANCH_TAKEN
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)printip, 
            IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    #ifdef Print2File
    fprintf(trace, "#eof\n");
    fclose(trace);
    #endif
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool works on branch prediction\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    trace = fopen("brtrace.out", "w");
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
