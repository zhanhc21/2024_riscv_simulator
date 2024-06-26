#include "cache-exp.h"
#include "processor.h"
#include "runner.h"

unsigned MeasureCacheBlockSize(ProcessorAbstract *p) {
    // TODO: Measure the cacheline size (or block size) of the cache in the given processor
    // The size will be ranged from 4 bytes to 32 bytes, and must be a power of 2
    // Return the accurate value

    // 步长
    unsigned stepSize[6] = {2, 4, 8, 16, 32, 64};
    unsigned testTime[6];

    for (int i = 0; i < 6; i++) {
        testTime[i] = execute(p, "./test/block_size", 1, stepSize[i]);
    }

    for (int i = 0; i < 6; i++) {
        Logger::Warn(
            "With step = %u, program simulator ran %u cycles.",
            stepSize[i],
            testTime[i]);
    }

    printf("\n");

    int mx = -1;
    int idx = 0;

    for (int i = 0; i < 5; i++) {
        int delta = ((int) testTime[i + 1]) - ((int) testTime[i]);
        if (delta > mx) {
            mx = delta;
            idx = i;
        }
    }
    return stepSize[idx];
}
