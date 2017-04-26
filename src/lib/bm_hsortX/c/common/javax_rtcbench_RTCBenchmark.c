#include <stdint.h>
#include "darjeeling3.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t count, int16_t a[]) {
	javax_rtc_RTC_void_startBenchmarkMeasurement_Native();

	// Then sort it
    int start, end;
 
    /* heapify */
    for (start = (count-2)/2; start >=0; start--) {
        // siftDown( a, start, count);
        // void siftDown( int16_t *a, int start, int count)
        // {
        int root = start;
     
        while ( root*2+1 < count ) {
            int child = 2*root + 1;
            if ((child + 1 < count) && (a[child] < a[child+1])) {
                child += 1;
            }
            if (a[root] < a[child]) {
                // SWAP( a[child], a[root] );
                {int16_t t=a[child]; a[child]=a[root]; a[root]=t; }

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
        {int16_t t=a[end]; a[end]=a[0]; a[0]=t; }

        // siftDown(a, 0, end);
        // void siftDown( int16_t *a, int start, int end)
        // {
        int root = 0;
     
        while ( root*2+1 < end ) {
            int child = 2*root + 1;
            if ((child + 1 < end) && (a[child] < a[child+1])) {
                child += 1;
            }
            if (a[root] < a[child]) {
                // SWAP( a[child], a[root] );
                {int16_t t=a[child]; a[child]=a[root]; a[root]=t; }

                root = child;
            }
            else
                break;
        }
        // }
    }
	javax_rtc_RTC_void_stopBenchmarkMeasurement();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	uint16_t NUMNUMBERS = 256;
	int16_t numbers[NUMNUMBERS];

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	rtcbenchmark_measure_native_performance(NUMNUMBERS, numbers);
}
