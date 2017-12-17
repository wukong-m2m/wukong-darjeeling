#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(int16_t NUMNUMBERS, uint32_t numbers[]) {
	rtc_startBenchmarkMeasurement_Native();

	// Then sort it
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		// uint16_t x=NUMNUMBERS-i-1; // Somehow this makes it a little slower at -Os. I expected no difference.
		for (uint16_t j=0; j<NUMNUMBERS-i-1; j++) {
			if (numbers[j]>numbers[j+1]) {
				uint32_t temp = numbers[j];
				numbers[j] = numbers[j+1];
				numbers[j+1] = temp;
			}
		}
	}

	rtc_stopBenchmarkMeasurement();
}
