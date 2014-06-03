/**
 * Glue code for running the AvDark cache simulator in Pin.
 */

#define Print2File

#include "pin.H"

#include <iostream>
#include <fstream>

#include <sys/time.h>

// lib
#include <string.h>
#include <stdlib.h>

 extern "C" {
#include "cache.h"
 }

 KNOB<string> knob_output(KNOB_MODE_WRITEONCE,    "pintool",
   "o", "cache.out", "specify log file name");
 KNOB<UINT32> knob_cache_mode(KNOB_MODE_WRITEONCE, "pintool",
   "cm", "1", "L2 Cache mode, 0 for inclusive, 1 for exclusive");

 KNOB<UINT32> knob_size_L1(KNOB_MODE_WRITEONCE, "pintool",
   "s1", "16384", "L1 Cache size (bytes)");
 KNOB<UINT32> knob_associativity_L1(KNOB_MODE_WRITEONCE, "pintool",
    "a1", "1", "L1 Cache associativity");
 KNOB<UINT32> knob_line_size_L1(KNOB_MODE_WRITEONCE, "pintool",
     "l1", "64", "L1 Cache line size");
 KNOB<string> knob_replacement_policy_L1(KNOB_MODE_WRITEONCE,"pintool",
    "replace1","LRU","L1 Cache replacement policy");

 KNOB<UINT32> knob_size_L2(KNOB_MODE_WRITEONCE, "pintool",
   "s2", "262144", "L2 Cache size (bytes), set 0 to turn it off");
 KNOB<UINT32> knob_associativity_L2(KNOB_MODE_WRITEONCE, "pintool",
    "a2", "1", "L2 Cache associativity");
 KNOB<UINT32> knob_line_size_L2(KNOB_MODE_WRITEONCE, "pintool",
     "l2", "64", "L2 Cache line size");
 KNOB<string> knob_replacement_policy_L2(KNOB_MODE_WRITEONCE,"pintool",
    "replace2","LRU","L2 Cache replacement policy");

 static avdark_cache_t *avdc_L1 = NULL;
 static avdark_cache_t *avdc_L2 = NULL;

 static int cache_mode = 0;
 static int state_total_data_read_miss = 0;
 static int state_total_data_write_miss = 0;

/**
 * Memory access callback. Will be called for every memory access
 * executed by the the target application.
 */
 static VOID
 simulate_access(VOID *addr, UINT32 access_type)
 {

        /* NOTE: We deliberately ignore the fact that addr may
         * straddle multiple cache lines */
    if(avdc_L2){
       if(cache_mode){
                //exclusive
          if(!avdc_access(avdc_L1, (avdc_pa_t)addr, (avdc_access_type_t)access_type)){
             if(avdc_access(avdc_L2, (avdc_pa_t)addr, (avdc_access_type_t)access_type)){
                        //remove required cache line from L2
                avdc_revoke(avdc_L2, (avdc_pa_t)addr);
                        //add L1 victim to L2
                avdc_access(avdc_L2, avdc_L1->last_victim, (avdc_access_type_t)access_type);
                
            }else{
                        //add L1 victim to L2
                avdc_access(avdc_L2, avdc_L1->last_victim, (avdc_access_type_t)access_type);
                
                        //record miss
                switch (access_type) {
                        case AVDC_READ: /* Read accesses */
                   state_total_data_read_miss++;
                   break;
                        case AVDC_WRITE: /* Write accesses */
                   state_total_data_write_miss++;
                   break;
               }
           }
       }            
   }else{   
                //inclusive
      if(!avdc_access(avdc_L1, (avdc_pa_t)addr, (avdc_access_type_t)access_type)){
         if(!avdc_access(avdc_L2, (avdc_pa_t)addr, (avdc_access_type_t)access_type)){
                        //make sure inclusive
			if(avdc_L2->last_valid)
	            avdc_revoke(avdc_L1, avdc_L2->last_victim);
            
                        //record miss
            switch (access_type) {
                        case AVDC_READ: /* Read accesses */
               state_total_data_read_miss++;
               break;
                        case AVDC_WRITE: /* Write accesses */
               state_total_data_write_miss++;
               break;
           }
       }
   }            
}
}else{
   avdc_access(avdc_L1, (avdc_pa_t)addr, (avdc_access_type_t)access_type);
}
}

/**
 * PIN instrumentation callback, called for every new instruction that
 * PIN discovers in the application. This function is used to
 * instrument code blocks by inserting calls to instrumentation
 * functions.
 */
 static VOID
 instruction(INS ins, VOID *not_used)
 {
    UINT32 no_ops = INS_MemoryOperandCount(ins);

    for (UINT32 op = 0; op < no_ops; op++) {
                //const UINT32 size = INS_MemoryOperandSize(ins, op);
                //const bool is_rd = INS_MemoryOperandIsRead(ins, op);
        const bool is_wr = INS_MemoryOperandIsWritten(ins, op);
        const UINT32 atype = is_wr ? AVDC_WRITE : AVDC_READ;


        INS_InsertCall(ins, IPOINT_BEFORE,
         (AFUNPTR)simulate_access,
         IARG_MEMORYOP_EA, op,
         IARG_UINT32, atype,
         IARG_END); 
    }
}

/**
 * PIN fini callback. Called after the target application has
 * terminated. Used to print statistics and do cleanup.
 */
 static VOID
 fini(INT32 code, VOID *v)
 {
    int accesses = avdc_L1->stat_data_read + avdc_L1->stat_data_write;
    int misses = avdc_L1->stat_data_read_miss + avdc_L1->stat_data_write_miss;
    int total_misses = state_total_data_read_miss + state_total_data_write_miss;
    
    avdc_delete(avdc_L1);
    if(avdc_L2)
       avdc_delete(avdc_L2);

#ifdef Print2File
   std::ofstream fileout(knob_output.Value().c_str());
   std::ostream *out = &fileout;
#else
   std::ostream *out = &std::cout;
#endif

   if(avdc_L2)
   {
     *out << "Cache statistics:" << endl;
     *out << "  Reads: " << avdc_L1->stat_data_read << endl;
     *out << "  L1 Read Misses: " << avdc_L1->stat_data_read_miss << endl;
     *out << "  Total Read Misses: " << state_total_data_read_miss << endl;
     *out << "  Writes: " << avdc_L1->stat_data_write << endl;
     *out << "  L1 Write Misses: " << avdc_L1->stat_data_write_miss << endl;
     *out << "  Total Write Misses: " << state_total_data_write_miss << endl;
     *out << "  Accesses: " << accesses << endl;
     *out << "  L1 Misses: " << misses << endl;
     *out << "  L1 Miss Ratio: " << ((100.0 * misses) / accesses) << "%" << endl;
     *out << "  Total Misses: " << total_misses << endl;
     *out << "  Total Miss Ratio: " << ((100.0 * total_misses) / accesses) << "%" << endl;
 }else{
   *out << "Cache statistics:" << endl;
   *out << "  Writes: " << avdc_L1->stat_data_write << endl;
   *out << "  Write Misses: " << avdc_L1->stat_data_write_miss << endl;
   *out << "  Reads: " << avdc_L1->stat_data_read << endl;
   *out << "  Read Misses: " << avdc_L1->stat_data_read_miss << endl;
   *out << "  Misses: " << misses << endl;
   *out << "  Accesses: " << accesses << endl;
   *out << "  Miss Ratio: " << ((100.0 * misses) / accesses) << "%" << endl;
}
}

static int
usage()
{
    cerr <<
    "This is the Advanced Computer Architecture online analysis tool\n"
    "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv))
        return usage();
    
    cache_mode = knob_cache_mode.Value();
    avdc_size_t size_L1 = knob_size_L1.Value();
    avdc_size_t size_L2 = knob_size_L2.Value();
    avdc_block_size_t block_size_L1 = knob_line_size_L1.Value();
    avdc_block_size_t block_size_L2 = knob_line_size_L2.Value();
    avdc_assoc_t assoc_L1 = knob_associativity_L1.Value();
    avdc_assoc_t assoc_L2 = knob_associativity_L2.Value();        
    avdc_replacement_t replacement_L1;
    avdc_replacement_t replacement_L2;
    replacement_L1 = (char*)malloc(strlen(knob_replacement_policy_L1.Value().c_str())*sizeof(char));
    replacement_L2 = (char*)malloc(strlen(knob_replacement_policy_L2.Value().c_str())*sizeof(char));
    strcpy(replacement_L1, knob_replacement_policy_L1.Value().c_str());
    strcpy(replacement_L2, knob_replacement_policy_L2.Value().c_str());

    avdc_L1 = avdc_new(size_L1, block_size_L1, assoc_L1, replacement_L1);
    avdc_L2 = avdc_new(size_L2, block_size_L2, assoc_L2, replacement_L2);
    if (!avdc_L1 || (!avdc_L2 && size_L2)) {
        cerr << "Failed to initialize the AvDark cache simulator." << endl;
        return -1;
    }

    INS_AddInstrumentFunction(instruction, 0);
    PIN_AddFiniFunction(fini, 0);

    PIN_StartProgram();
    return 0;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * indent-tabs-mode: nil
 * c-file-style: "linux"
 * compile-command: "make -k -C ../../"
 * End:
 */
