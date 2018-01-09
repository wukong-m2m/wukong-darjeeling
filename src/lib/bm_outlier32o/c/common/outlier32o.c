#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "rtc_measure.h"

// From the Harbor paper
//
// "The outlier detector samples a set of sensor values and stores them in a buffer. Once the buffer is filled, it
// computes the distance between all pairs of samples in the buffer and stores the result in a matrix. Using a
// distance threshold, the algorithm marks the distance measurements in the matrix that are greater than the 
// threshold. If the majority of the distance measurements for a sensor readings are marked, then the sensor reading
// is classified as an outlier."
//
// There's no reason to store the results in a matrix. This version just counts distances exceeding the threshold as we scan the buffer.

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int32_t buffer[], int32_t distance_threshold, bool outliers[]) {
	rtc_startBenchmarkMeasurement_Native();

	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		uint16_t exceed_threshold_count = 0;
		for (uint16_t j=0; j<NUMNUMBERS; j++) {
			int32_t diff = buffer[i] - buffer[j];
			if (diff > distance_threshold || -diff > distance_threshold) {
				exceed_threshold_count++;
			}
		}
		if (exceed_threshold_count > (NUMNUMBERS / 2)) {
			outliers[i] = true;
		} else {
			outliers[i] = false;
		}
	}

	rtc_stopBenchmarkMeasurement();
}