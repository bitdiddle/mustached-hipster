/**
 * Cache simulation using a functional system simulator.
 */

#include "cache.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
 
#define AVDC_MALLOC(nelems, type) malloc(nelems * sizeof(type))
#define AVDC_FREE(p) free(p)

/**
 * Cache block information.
 *
 * HINT: You will probably need to change this structure
 */
struct avdc_cache_line {
        avdc_tag_t tag;
        //avdc_index_t index;
        long        count;
        int        valid;
};

/**
 * Extract the cache line tag from a physical address.
 *
 * You probably don't want to change this function, instead you may
 * want to change how the tag_shift field is calculated in
 * avdc_resize().
 */
static inline avdc_pa_t
tag_from_pa(avdark_cache_t *self, avdc_pa_t pa)
{
        return pa >> self->tag_shift;
}

/**
 * Calculate the cache line index from a physical address.
 *
 * Feel free to experiment and change this function
 */
static inline int
index_from_pa(avdark_cache_t *self, avdc_pa_t pa)
{
        return (pa >> self->block_size_log2) & (self->number_of_sets - 1);
}

/**
 * Computes the log2 of a 32 bit integer value. Used in dc_init
 *
 * Do NOT modify!
 */
static int
log2_int32(uint32_t value)
{
        int i;

        for (i = 0; i < 32; i++) {
                value >>= 1;
                if (value == 0)
                        break;
        }
        return i;
}

/**
 * Check if a number is a power of 2. Used for cache parameter sanity
 * checks.
 *
 * Do NOT modify!
 */
static int
is_power_of_two(uint64_t val)
{
        return ((((val)&(val-1)) == 0) && (val > 0));
}

void
avdc_dbg_log(avdark_cache_t *self, const char *msg, ...)
{
        va_list ap;
        
        if (self->dbg) {
                const char *name = self->dbg_name ? self->dbg_name : "AVDC";
                fprintf(stderr, "[%s] dbg: ", name);
                va_start(ap, msg);
                vfprintf(stderr, msg, ap);
                va_end(ap);
        }
}


void
avdc_access(avdark_cache_t *self, avdc_pa_t pa, avdc_access_type_t type)
{
        self->count++;
        /* HINT: You will need to update this function */
        avdc_tag_t tag = tag_from_pa(self, pa);
        int index = index_from_pa(self, pa);
        int hit = 0;

        if (strcmp(self->replacement,"LRU") == 0)
        {
            int i;
            for (i = 0; i < self->assoc ; i++)
            {
                int index_temp = self->assoc * index + i;
                if (self->lines[index_temp].valid && self->lines[index_temp].tag == tag){
                    hit = 1;
                    self->lines[index_temp].count = self->count;
                    break;
                }        
            }
            
            long min = self->count;
            int lru_index;
            if (!hit) {
                for (i = 0; i < self->assoc ; i++)
                {
                    int index_temp = self->assoc * index + i;
                    if (self->lines[index_temp].count < min)
                    {
                        min = self->lines[index_temp].count;
                        lru_index = index_temp;    
                    }
                }   
                self->lines[lru_index].valid = 1;
                self->lines[lru_index].tag = tag;
                self->lines[lru_index].count = self->count;
            } 
        }
        
        if (strcmp(self->replacement,"RANDOM") == 0)
        {
            int i;
            for (i = 0; i < self->assoc ; i++)
            {
                int index_temp = self->assoc * index + i;
                if (self->lines[index_temp].valid && self->lines[index_temp].tag == tag){
                    hit = 1;
                    break;
                }        
            }

            if (!hit)
            {
                int random_index = rand() % self->assoc;
                self->lines[random_index].valid = 1;
                self->lines[random_index].tag = tag;        
            }
        }

        if (strcmp(self->replacement,"FIFO") == 0)
        {
            int i;
            for (i = 0; i < self->assoc ; i++)
            {
                int index_temp = self->assoc * index + i;
                if (self->lines[index_temp].valid && self->lines[index_temp].tag == tag){
                    hit = 1;
                    break;
                }        
            }

            if (!hit){
                int fifo_index = self->assoc * index + self->head[index];
                self->head[index]++;
                self->lines[fifo_index].valid = 1;
                self->lines[fifo_index].tag = tag;      
            }
        }

        switch (type) {
        case AVDC_READ: /* Read accesses */
                avdc_dbg_log(self, "read: pa: 0x%.16lx, tag: 0x%.16lx, index: %d, hit: %d\n",
                             (unsigned long)pa, (unsigned long)tag, index, hit);
                self->stat_data_read += 1;
                if (!hit)
                        self->stat_data_read_miss += 1;
                break;

        case AVDC_WRITE: /* Write accesses */
                avdc_dbg_log(self, "write: pa: 0x%.16lx, tag: 0x%.16lx, index: %d, hit: %d\n",
                             (unsigned long)pa, (unsigned long)tag, index, hit);
                self->stat_data_write += 1;
                if (!hit)
                        self->stat_data_write_miss += 1;
                break;
        }
}

void
avdc_flush_cache(avdark_cache_t *self)
{
	int i;
        /* HINT: You will need to update this function */
        for (i = 0; i < self->number_of_sets * self->assoc; i++) 
        {
                self->lines[i].valid = 0;
                self->lines[i].tag = 0;
                self->lines[i].count = 0;       
        }
        srand(time(NULL));
        for(i = 0; i < self->number_of_sets; i++)
        {
            self->head[i] = 0;
            //self->full[i] = false;
        }
}


int
avdc_resize(avdark_cache_t *self,
            avdc_size_t size, avdc_block_size_t block_size, avdc_assoc_t assoc)
{
        /* HINT: This function precomputes some common values and
         * allocates the self->lines array. You will need to update
         * this to reflect any changes to how this array is supposed
         * to be allocated.
         */

        /* Verify that the parameters are sane */
        if (!is_power_of_two(size) ||
            !is_power_of_two(block_size) ||
            !is_power_of_two(assoc)) {
                fprintf(stderr, "size, block-size and assoc all have to be powers of two and > zero\n");
                return 0;
        }

        /* Update the stored parameters */
        self->size = size;
        self->block_size = block_size;
        self->assoc = assoc;

        /* Cache some common values */
        self->number_of_sets = (self->size / self->block_size) / self->assoc;
        self->block_size_log2 = log2_int32(self->block_size);
        self->tag_shift = self->block_size_log2 + log2_int32(self->number_of_sets);
        
        //printf("%d %d %d %d\n",self->assoc,self->number_of_sets,self->block_size_log2,self->tag_shift);  
        /* (Re-)Allocate space for the tags array */
        
        if (self->lines)
            AVDC_FREE(self->lines);
        if (self->head)
            AVDC_FREE(self->head);
        //if (self->full)
        //    AVDC_FREE(self->full);
        
        /* HINT: If you change this, you may have to update
         * avdc_delete() to reflect changes to how thie self->lines
         * array is allocated. */
        self->lines = AVDC_MALLOC(self->number_of_sets * self->assoc, avdc_cache_line_t);
        self->head = AVDC_MALLOC(self->number_of_sets , int);
        //self->lines = AVDC_MALLOC(self->number_of_sets , bool);
       
        /* Flush the cache, this initializes the tag array to a known state */
         
        avdc_flush_cache(self);

        return 1;
}

void
avdc_print_info(avdark_cache_t *self)
{
        fprintf(stderr, "Cache Info\n");
        fprintf(stderr, "size: %d, assoc: %d, line-size: %d\n",
                self->size, self->assoc, self->block_size);
}

void
avdc_print_internals(avdark_cache_t *self)
{
        int i;

        fprintf(stderr, "Cache Internals\n");
        fprintf(stderr, "size: %d, assoc: %d, line-size: %d\n",
                self->size, self->assoc, self->block_size);

        for (i = 0; i < self->number_of_sets; i++)
                fprintf(stderr, "tag: <0x%.16lx> valid: %d\n",
                        (long unsigned int)self->lines[i].tag,
                        self->lines[i].valid);
}

void
avdc_reset_statistics(avdark_cache_t *self)
{
        self->stat_data_read = 0;
        self->stat_data_read_miss = 0;
        self->stat_data_write = 0;
        self->stat_data_write_miss = 0;
}

avdark_cache_t *
avdc_new(avdc_size_t size, avdc_block_size_t block_size,
         avdc_assoc_t assoc,avdc_replacement_t replacement)
{
        avdark_cache_t *self;

        self = AVDC_MALLOC(1, avdark_cache_t);

        memset(self, 0, sizeof(*self));
        self->dbg = 0;
        self->count = 0;
        self->replacement = (char*)malloc(strlen(replacement)*sizeof(char));
        strcpy(self->replacement,replacement);

        if (!avdc_resize(self, size, block_size, assoc)) {
                AVDC_FREE(self);
                return NULL;
        }

        return self;
}

void
avdc_delete(avdark_cache_t *self)
{
        if (self->lines)
                AVDC_FREE(self->lines);

        AVDC_FREE(self);
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
