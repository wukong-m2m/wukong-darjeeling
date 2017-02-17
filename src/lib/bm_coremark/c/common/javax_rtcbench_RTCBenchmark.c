#include <stdint.h>
#include "darjeeling3.h"
#include "config.h"

// Split into separate function to avoid the compiler just optimising away the whole test.


void __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	avroraPrintStr("C version runs in separate conf");

	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	rtcbenchmark_measure_native_performance();
}
