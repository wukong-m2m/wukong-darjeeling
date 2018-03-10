#include <stdint.h>
#include "config.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint32_t *v, uint8_t n, uint32_t const key[4]);

void javax_rtcbench_RTCBenchmark_void_test_native() {
	uint8_t NUMNUMBERS = 32;
	uint32_t numbers[NUMNUMBERS];
	uint32_t const key[4] = {0, 1, 2, 3};

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	rtcbenchmark_measure_native_performance(numbers, NUMNUMBERS, key);

	for (uint16_t i=0; i<NUMNUMBERS; i++)
	    avroraPrintInt32((int32_t)numbers[i]);
	avroraPrintStr("done.");
}

