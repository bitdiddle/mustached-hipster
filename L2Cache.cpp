#include <iostream>
#include <fstream>

#include <sys/time.h>

// lib
#include <string.h>
#include <stdlib.h>

extern "C" {
	#include "cache.h"
}

using namespace std;

enum Mode { Exclusive, Inclusive };

class L2Cache
{
public:
	Mode cache_mode;
	unsigned long state_total_data_read_miss, state_total_data_write_miss;
	avdark_cache_t *avdc_L1;
	avdark_cache_t *avdc_L2;

public:
	L2Cache(
		avdc_size_t size_L1, avdc_block_size_t block_size_L1, 
		avdc_assoc_t assoc_L1, avdc_replacement_t replacement_L1,
		avdc_size_t size_L2, avdc_block_size_t block_size_L2, 
		avdc_assoc_t assoc_L2, avdc_replacement_t replacement_L2,
		Mode mode
		)
	{
		cache_mode = mode;
		state_total_data_read_miss = 0;
		state_total_data_write_miss = 0;
		avdc_L1 = avdc_new(size_L1, block_size_L1, assoc_L1, replacement_L1);
		avdc_L2 = avdc_new(size_L2, block_size_L2, assoc_L2, replacement_L2);
		if (!avdc_L1 || !avdc_L2) {
			cerr << "Failed to initialize the AvDark cache simulator." << endl;
			exit(0);
		}
	};

	void access(avdc_pa_t addr, avdc_access_type_t access_type)
	{
		if(cache_mode == Exclusive){
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
		}
		if(cache_mode == Inclusive){
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
	};


	~L2Cache()
	{
		avdc_delete(avdc_L1);
		avdc_delete(avdc_L2);
		// cout<<"read miss: "<<state_total_data_read_miss<<endl;
		// cout<<"write miss: "<<state_total_data_write_miss<<endl;
	};
};
