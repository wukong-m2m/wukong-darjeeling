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

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint16_t NUMNUMBERS, int16_t buffer[], uint16_t distance_matrix[], uint16_t distance_threshold, bool outliers[]) {
	rtc_startBenchmarkMeasurement_Native();

    // Calculate distance matrix
    uint16_t sub_start=0;
    for (uint16_t i=0; i<NUMNUMBERS; i++) {
        uint16_t hor = sub_start;
        uint16_t ver = sub_start;
        for (uint16_t j=i; j<NUMNUMBERS; j++) {
            if (buffer[i] > buffer[j]) {
                uint16_t diff = (uint16_t)(buffer[i] - buffer[j]);
                distance_matrix[hor] = diff;
                distance_matrix[ver] = diff;
            } else {
                uint16_t diff = (uint16_t)(buffer[j] - buffer[i]);
                distance_matrix[hor] = diff;
                distance_matrix[ver] = diff;
            }

            hor ++;
            ver += NUMNUMBERS;
        }
        sub_start+=NUMNUMBERS+1;
    }
    
    // Determine outliers
    uint16_t k=0; // Since we scan one line at a time, we don't need to calculate a matrix index.
                  // The first NUMNUMBERS distances correspond to measurement 1, the second NUMNUMBERS distances to measurement 2, etc.
    for (uint16_t i=0; i<NUMNUMBERS; i++) {
        uint16_t exceed_threshold_count = 0;
        for (uint16_t j=0; j<NUMNUMBERS; j++) {
            if (distance_matrix[k++] > distance_threshold) {
                exceed_threshold_count++;
            }
        }
        if (exceed_threshold_count > (NUMNUMBERS >> 1)) {
            outliers[i] = true;
        } else {
            outliers[i] = false;
        }
    }

	rtc_stopBenchmarkMeasurement();
}