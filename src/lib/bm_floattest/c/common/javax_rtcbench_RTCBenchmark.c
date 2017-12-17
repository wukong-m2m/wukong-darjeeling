#include <stdint.h>
#include "darjeeling3.h"
#include "config.h"
#include "rtc_measure.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

float __attribute__((noinline)) add(float a, float b) {
	return a+b;
}

float __attribute__((noinline)) substract(float a, float b) {
	return a-b;
}

float __attribute__((noinline)) multiply(float a, float b) {
	return a*b;
}

float __attribute__((noinline)) divide(float a, float b) {
	return a/b;
}

float __attribute__((noinline)) negative(float a) {
	return -a;
}

void __attribute__((noinline)) rtcbenchmark_measure_native_performance(float a, float b) {
	rtc_startBenchmarkMeasurement_Native();

	// Hopefully enough to prevent the optimiser from removing all of this.
	if (add(a, b) + substract(a, b) + multiply(a, b) + divide(a, b) + negative(a) < 0) {
		avroraPrintInt32(-1);
	} else {
		avroraPrintInt32(1);		
	}

	rtc_stopBenchmarkMeasurement();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	rtcbenchmark_measure_native_performance(0.0000000001, 1000000000.0);
}
