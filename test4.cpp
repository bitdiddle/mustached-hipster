/**
 * Cache simulator test case - 2 level cache
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "L2Cache.cpp"

#define makesure assert
// #define makesure cout<<

int
main(int argc, char *argv[])
{
        printf("L2 Cache, inclusive: read\n");
        L2Cache l2cir(512, 64, 8, "LRU", 4096, 64, 8, "LRU", Inclusive);
        // fill L1
        for (int i = 0; i < 8; ++i)
        {
        	l2cir.access(i*64, AVDC_READ);
        };
        // check
        for (int i = 0; i < 8; ++i)
        {
        	makesure(avdc_access(l2cir.avdc_L1, i*64, AVDC_READ));
        	makesure(avdc_access(l2cir.avdc_L2, i*64, AVDC_READ));
        };

        printf("L2 Cache, inclusive: write\n");
        L2Cache l2ciw(512, 64, 8, "LRU", 4096, 64, 8, "LRU", Inclusive);
        // fill L1
        for (int i = 0; i < 8; ++i)
        {
        	l2ciw.access(i*64, AVDC_WRITE);
        };
        // check
        for (int i = 0; i < 8; ++i)
        {
        	makesure(avdc_access(l2ciw.avdc_L1, i*64, AVDC_WRITE));
        	makesure(avdc_access(l2ciw.avdc_L2, i*64, AVDC_WRITE));
        };

		printf("L2 Cache, exclusive: read\n");
        L2Cache l2cer(512, 64, 8, "LRU", 4096, 64, 8, "LRU", Exclusive);
        // fill L1
        for (int i = 0; i < 8; ++i)
        {
        	l2cer.access(i*64, AVDC_READ);
        };
        // kick original entries
        for (int i = 0; i < 8; ++i)
        {
        	l2cer.access(i*64 + 512, AVDC_READ);
        };
        // check
        for (int i = 0; i < 8; ++i)
        {
        	makesure(!avdc_access(l2cer.avdc_L1, i*64, AVDC_READ));
        	makesure(avdc_access(l2cer.avdc_L2, i*64, AVDC_READ));
        };

        printf("L2 Cache, exclusive: write\n");
        L2Cache l2cew(512, 64, 8, "LRU", 4096, 64, 8, "LRU", Exclusive);
        // fill L1
        for (int i = 0; i < 8; ++i)
        {
        	l2cew.access(i*64, AVDC_WRITE);
        };
        // kick original entries
        for (int i = 0; i < 8; ++i)
        {
        	l2cew.access(i*64 + 512, AVDC_WRITE);
        };
        // check
        for (int i = 0; i < 8; ++i)
        {
        	makesure(!avdc_access(l2cew.avdc_L1, i*64, AVDC_WRITE));
        	makesure(avdc_access(l2cew.avdc_L2, i*64, AVDC_WRITE));
        };

        printf("%s done.\n", argv[0]);
        return 0;
}
