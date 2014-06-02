/**
 * Cache simulator test case - 2 level cache
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

int
main(int argc, char *argv[])
{
        avdark_cache_t *cache;

        printf("\nFully associative, LRU\n");

        cache = avdc_new(512, 64, 8, "LRU");
        assert(cache);
        avdc_print_info(cache);

        printf("Aliasing [read]\n");
        test(cache, 64, AVDC_READ);
        printf("Aliasing [write]\n");
        avdc_flush_cache(cache);
        test(cache, 64, AVDC_WRITE);

 
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
