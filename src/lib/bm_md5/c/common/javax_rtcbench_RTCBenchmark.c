#include <stdint.h>
#include "darjeeling3.h"
#include "debug.h"
#include "xxtea.h"

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

// Split into separate function to avoid the compiler just optimising away the whole test.
void rtcbenchmark_measure_native_performance(uint32_t *v, uint8_t n, uint32_t const key[4]) {
	javax_darjeeling_Stopwatch_void_resetAndStart();



	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {

	rtcbenchmark_measure_native_performance(numbers, NUMNUMBERS, key);

}

