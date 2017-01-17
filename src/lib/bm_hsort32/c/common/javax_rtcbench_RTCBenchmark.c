#include <stdint.h>
#include "darjeeling3.h"
#include "heap.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t count, int32_t a[]) {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	// Then sort it
    int start, end;
 
    /* heapify */
    for (start = (count-2)/2; start >=0; start--) {
        // siftDown( a, start, count);
        // void siftDown( int32_t *a, int start, int count)
        // {
        int root = start;
        int child;
        while ( (child = (root << 1)+1) < count ) {
            int child_plus_one = child + 1;
            if ((child_plus_one < count) && (a[child] < a[child_plus_one])) {
                child += 1;
            }
            int32_t a_root = a[root];
            int32_t a_child = a[child];
            if (a[root] < a[child]) {
                // SWAP( a[child], a[root] );
                a[root] = a_child;
                a[child] = a_root;

                root = child;
            }
            else {
                break;
            }
        }
        // }
    }
 
    for (end=count-1; end > 0; end--) {
        // SWAP(a[end],a[0]);
        {int32_t t=a[end]; a[end]=a[0]; a[0]=t; }

        // siftDown(a, 0, end);
        // void siftDown( int32_t *a, int start, int end)
        // {
        int root = 0;
        int child;
        while ( (child = (root << 1)+1) < end ) {
            int child_plus_one = child + 1;
            if ((child_plus_one < end) && (a[child] < a[child_plus_one])) {
                child += 1;
            }
            int32_t a_root = a[root];
            int32_t a_child = a[child];
            if (a[root] < a[child]) {
                // SWAP( a[child], a[root] );
                a[root] = a_child;
                a[child] = a_root;

                root = child;
            }
            else
                break;
        }
        // }
    }
	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	uint16_t NUMNUMBERS = 256;
    int32_t *numbers = dj_mem_alloc(NUMNUMBERS*sizeof(int32_t), CHUNKID_RTCNATIVETESTDATA);

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	rtcbenchmark_measure_native_performance(NUMNUMBERS, numbers);
}
