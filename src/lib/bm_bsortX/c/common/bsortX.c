#include <stdint.h>
#include "config.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t numbers[]) {
	javax_rtc_RTC_void_startBenchmarkMeasurement_Native();

	// Then sort it
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		// uint16_t x=NUMNUMBERS-i-1; // Somehow this makes it a little slower at -Os. I expected no difference.
		for (uint16_t j=0; j<NUMNUMBERS-i-1; j++) {
			if (numbers[j]>numbers[j+1]) {
				int16_t temp = numbers[j];
				numbers[j] = numbers[j+1];
				numbers[j+1] = temp;
			}
		}
	}

	javax_rtc_RTC_void_stopBenchmarkMeasurement();
}
