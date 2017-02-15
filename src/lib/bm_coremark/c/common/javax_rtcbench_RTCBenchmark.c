#include <stdint.h>
#include "darjeeling3.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

// The main function in the Core Mark code
void core_mark_main(void);

void __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	core_mark_main();

	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	rtcbenchmark_measure_native_performance();
}
