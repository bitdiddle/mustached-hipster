/**
 * Cache simulator test case - replacement policy RANDOM, FIFO 
 */

#include "cache.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define STAT_ASSERT(c, r, rm, w, wm) do {                       \
                assert(c->stat_data_read == (r));               \
                assert(c->stat_data_read_miss == (rm));         \
                assert(c->stat_data_write == (w));              \
                assert(c->stat_data_write_miss == (wm));        \
        } while (0)

#define TEST_SIMPLE_STAT() \
        if (type == AVDC_READ)                                  \
                STAT_ASSERT(cache, hits+misses, misses, 0, 0);   \
        else if (type == AVDC_WRITE)                            \
                STAT_ASSERT(cache, 0, 0, hits+misses, misses);  \
        else                                                    \
                abort()

static void
test_lru(avdark_cache_t *cache, avdc_pa_t alias_offset, avdc_access_type_t type)
{
        int i;
        int hits = 0;
        int misses = 0;

        avdc_reset_statistics(cache);
        STAT_ASSERT(cache, 0, 0, 0, 0);

        /* Access all ways in the first set, we should get 1 miss per way */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                misses++;
                TEST_SIMPLE_STAT();
        }

        /* Now, access all the ways again, we shouldn't get any misses */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                hits++;
                TEST_SIMPLE_STAT();
        }

        /* Access 1 cache line that aliases into set 0 and replaces
         * a line that we just loaded */
        avdc_access(cache, alias_offset * cache->assoc, type);
        misses++;
        TEST_SIMPLE_STAT();

        /* Access all lines except for the first one we touched (which
         * has been replaced by the LRU algorithm). We should only get
         * hits here. */
        for (i = 1; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                hits++;
                TEST_SIMPLE_STAT();
        }

        /* Now, access the first line again, this should be a miss */
        avdc_access(cache, 0, type);
        misses++;
        TEST_SIMPLE_STAT();
}
static void
test(avdark_cache_t *cache, avdc_pa_t alias_offset, avdc_access_type_t type)
{
        int i;
        int hits = 0;
        int misses = 0;

        avdc_reset_statistics(cache);
        STAT_ASSERT(cache, 0, 0, 0, 0);

        /* Access all ways in the first set, we should get 1 miss per way */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                misses++;
                TEST_SIMPLE_STAT();
        }

        /* Now, access all the ways again, we shouldn't get any misses */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                hits++;
                TEST_SIMPLE_STAT();
        }

        /* Access 1 cache line that aliases into set 0 and replaces
         * a line that we just loaded */
        avdc_access(cache, alias_offset * cache->assoc, type);
        misses++;
        TEST_SIMPLE_STAT();

        /* Access all lines except for the first one we touched (which
         * has been replaced by the LRU algorithm). We should only get
         * hits here. */
        for (i = 1; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                hits++;
                TEST_SIMPLE_STAT();
        }

        /* Now, access the first line again, this should be a miss */
        avdc_access(cache, 0, type);
        misses++;
        TEST_SIMPLE_STAT();
}

/*
 * test for random replacement policy,
 * anything can happen ...
 */
static void
test_random(avdark_cache_t *cache, avdc_pa_t alias_offset, avdc_access_type_t type)
{
        int i;
        int hits = 0;
        int misses = 0;

        avdc_reset_statistics(cache);
        STAT_ASSERT(cache, 0, 0, 0, 0);

        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type) ? hits++ : misses++;
                TEST_SIMPLE_STAT();
        }

        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type) ? hits++ : misses++;
                TEST_SIMPLE_STAT();
        }

        avdc_access(cache, alias_offset * cache->assoc, type) ? hits++ : misses++;
        TEST_SIMPLE_STAT();

        for (i = 1; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type) ? hits++ : misses++;
                TEST_SIMPLE_STAT();
        }

        avdc_access(cache, 0, type) ? hits++ : misses++;
        TEST_SIMPLE_STAT();
}

/*
 * test FIFO
 */
static void
test_fifo(avdark_cache_t *cache, avdc_pa_t alias_offset, avdc_access_type_t type)
{
        int i;
        int hits = 0;
        int misses = 0;

        avdc_reset_statistics(cache);
        STAT_ASSERT(cache, 0, 0, 0, 0);

        /* Access all ways in the first set, we should get 1 miss per way */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                misses++;
                TEST_SIMPLE_STAT();
        }

        /* Now, access all the ways again, we shouldn't get any misses */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                hits++;
                TEST_SIMPLE_STAT();
        }

        /* Access 1 cache line that aliases into set 0 and replaces the earlier ones*/
	// though i is accessed most recently, it's selected as victim because it's first in queue
	// 1 2 3 4
        avdc_access(cache, alias_offset * (0), type);
        hits++;
        avdc_access(cache, alias_offset * (cache->assoc + 0), type);
        misses++;
	// 2 3 4 5
        avdc_access(cache, alias_offset * (0), type);
        misses++;
	// 3 4 5 1
        TEST_SIMPLE_STAT();
        for (i = 1; i < cache->assoc; i++) {
        	avdc_access(cache, alias_offset * (i), type);
        	misses++;
		// 4 5 1 2
                avdc_access(cache, alias_offset * (i), type);
                hits++;
	}
        TEST_SIMPLE_STAT();
}

static void
test_roundrobin(avdark_cache_t *cache, avdc_pa_t alias_offset, avdc_access_type_t type)
{
        int i;
        int hits = 0;
        int misses = 0;

        avdc_reset_statistics(cache);
        STAT_ASSERT(cache, 0, 0, 0, 0);

        /* Access all ways in the first set, we should get 1 miss per way */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                misses++;
                TEST_SIMPLE_STAT();
        }

        /* Now, access all the ways again, we shouldn't get any misses */
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                hits++;
                TEST_SIMPLE_STAT();
        } 

        /* Access 1 cache line that aliases into set 0 and replaces the earlier ones*/
        for (i = 0; i < cache->assoc; i++) {
		// though i is accessed most recently, it's selected as victim under round-robin
        	avdc_access(cache, alias_offset * (i), type);
        	hits++;
        	avdc_access(cache, alias_offset * (cache->assoc + i), type);
                misses++;
	}
        TEST_SIMPLE_STAT();
        for (i = 0; i < cache->assoc; i++) {
                avdc_access(cache, alias_offset * i, type);
                misses++;
                TEST_SIMPLE_STAT();
        }
}

int
main(int argc, char *argv[])
{
        avdark_cache_t *cache;

        printf("\nLRU\n");

        cache = avdc_new(512, 64, 8, "LRU");
        assert(cache);
        avdc_print_info(cache);

        printf("[read]\n");
        test_lru(cache, 64, AVDC_READ);
        printf("[write]\n");
        avdc_flush_cache(cache);
        test_lru(cache, 64, AVDC_WRITE);
 
        avdc_delete(cache);

        printf("\nRANDOM\n");

        cache = avdc_new(512, 64, 8, "RANDOM");
        assert(cache);
        avdc_print_info(cache);

        printf("[read]\n");
        test_random(cache, 64, AVDC_READ);
        printf("[write]\n");
        avdc_flush_cache(cache);
        test_random(cache, 64, AVDC_WRITE);
 
        avdc_delete(cache);

        printf("\nFIFO\n");

        cache = avdc_new(512, 64, 8, "FIFO");
        assert(cache);
        avdc_print_info(cache);

        printf("[read]\n");
        test_fifo(cache, 64, AVDC_READ);
        printf("[write]\n");
        avdc_flush_cache(cache);
        test_fifo(cache, 64, AVDC_WRITE);
 
        avdc_delete(cache);

        printf("\nROUNDROBIN\n");

        cache = avdc_new(512, 64, 8, "ROUNDROBIN");
        assert(cache);
        avdc_print_info(cache);

        printf("[read]\n");
        test_roundrobin(cache, 64, AVDC_READ);
        printf("[write]\n");
        avdc_flush_cache(cache);
        test_roundrobin(cache, 64, AVDC_WRITE);
 
        avdc_delete(cache);

        printf("%s done.\n", argv[0]);
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
