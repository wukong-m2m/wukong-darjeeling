#include <stdint.h>
#include "config.h"
#include "rtc_measure.h"
#include <stdbool.h>


void __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
	rtc_startBenchmarkMeasurement_Native();



	rtc_stopBenchmarkMeasurement();
}