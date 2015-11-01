#include <stdint.h>
#include "config.h"
#include "darjeeling3.h"
#include "rc5.h"


// Split into separate function to avoid the compiler just optimising away the whole test.
void javax_rtcbench_RTCTestRC5_void_test_rc5_native2() {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	avroraPrintStr("rc5_test: ");
	int rv = rc5_test();
	avroraPrintInt32(rv);

	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCTestRC5_void_test_rc5_native() {
	javax_rtcbench_RTCTestRC5_void_test_rc5_native2();
}

