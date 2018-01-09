#include <stdint.h>
#include "config.h"
#include "heap.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance();

void javax_rtcbench_RTCBenchmark_void_test_native() {
	rtcbenchmark_measure_native_performance();
}
