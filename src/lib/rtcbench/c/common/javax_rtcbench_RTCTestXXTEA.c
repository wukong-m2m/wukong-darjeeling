#include <stdint.h>
#include "darjeeling3.h"
#include "debug.h"
#include "xxtea.h"

// Split into separate function to avoid the compiler just optimising away the whole test.
void javax_rtcbench_RTCTestXXTEA_void_test_xxtea_native2(uint32_t *v, int n, uint32_t const key[4]) {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	btea(v, n, key);

	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCTestXXTEA_void_test_xxtea_native() {
	uint8_t NUMNUMBERS = 32;
	uint32_t numbers[NUMNUMBERS];
	uint32_t const key[4] = {0, 1, 2, 3};

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	javax_rtcbench_RTCTestXXTEA_void_test_xxtea_native2(numbers, NUMNUMBERS, key);

	for (uint16_t i=0; i<NUMNUMBERS; i++)
		DEBUG_LOG(1, "%lu ", numbers[i]);
	DEBUG_LOG(1, "done.");
}

