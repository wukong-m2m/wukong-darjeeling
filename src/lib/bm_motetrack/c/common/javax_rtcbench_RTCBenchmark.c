#include <stdint.h>
#include "config.h"
#include "heap.h"

#include "motetrack.h"

void __attribute__((noinline)) rtcbenchmark_measure_native_performance();

void javax_rtcbench_RTCBenchmark_void_test_native() {
    motetrack_init_benchmark();
	rtcbenchmark_measure_native_performance();
}
