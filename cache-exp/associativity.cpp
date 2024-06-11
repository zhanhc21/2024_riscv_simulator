#include "cache-exp.h"
#include "processor.h"
#include "runner.h"

unsigned MeasureCacheAssociativity([[maybe_unused]] ProcessorAbstract *p,
                                   [[maybe_unused]] unsigned cacheSize,
                                   [[maybe_unused]] unsigned cacheBlockSize) {
    // TODO: Measure the associativiy of the cache in the given processor
    // The associativity will be ranged from 1 to 8, and must be a power of 2
    // Return the accurate value
    
    unsigned associate[6] = {1, 2, 4, 8, 16, 32};
    unsigned testTime[6];

    for (int i = 0; i < 6; i++) {
        testTime[i] = execute(p, "./test/associate", 2,  cacheSize, associate[i]);
    }

    for (int i = 0; i < 6; i++) {
        Logger::Warn(
            "With assoc = %u, program simulator ran %u cycles.",
            associate[i],
            testTime[i]);
    }

    printf("\n");

    int mx = -1;
    int idx = 0;

    for (int i = 0; i < 5; i++) {
        int delta = ((int) testTime[i + 1]) - ((int) testTime[i]);
        if (delta > mx) {
            mx = delta;
            idx = i + 1;
        }
    }
    return associate[idx];
}
