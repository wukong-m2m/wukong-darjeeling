#include <stdint.h>
#include "config.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t count, int16_t a[]);

void javax_rtcbench_RTCBenchmark_void_test_native() {
	uint16_t NUMNUMBERS = 256;
	int16_t numbers[NUMNUMBERS];

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	rtcbenchmark_measure_native_performance(NUMNUMBERS, numbers);
}
