#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, volatile int8_t numbers[]) {
	rtc_startBenchmarkMeasurement_Native();

	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		numbers[i] = 1;
	}

	rtc_stopBenchmarkMeasurement();
}