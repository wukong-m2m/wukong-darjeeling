#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t numbers[]) {
	rtc_startBenchmarkMeasurement_Native();

	// Then sort it
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		uint16_t x=(NUMNUMBERS-i-1); // This doesn't get optimised the way I expected it would. Without this extra variable, it will calculate NUMNUMBERS-i-1 on each interation of the inner loop! (speedup 14.7M -> 14.2M cycles)
		uint16_t j_plus_one = 1; // Same goes for "j+1"
		int16_t val_at_j_plus_one = numbers[0];
		// uint16_t x=NUMNUMBERS-i-1; // Somehow this makes it a little slower at -Os. I expected no difference.
		for (uint16_t j=0; j<x; j++) {
			int16_t val_at_j = numbers[j];
			val_at_j_plus_one = numbers[j_plus_one];
			if (val_at_j>val_at_j_plus_one) {
				numbers[j] = val_at_j_plus_one;
				numbers[j+1] = val_at_j;
				val_at_j_plus_one = val_at_j;
			}
			j_plus_one++;
		}
	}

	rtc_stopBenchmarkMeasurement();
}

