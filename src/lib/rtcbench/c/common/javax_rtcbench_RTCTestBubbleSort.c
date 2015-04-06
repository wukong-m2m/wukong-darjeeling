#include <stdint.h>
#include "darjeeling3.h"

// Split into separate function to avoid the compiler just optimising away the whole test.

void javax_rtcbench_RTCTestBubbleSort_void_test_bubblesort_native2(uint16_t NUMNUMBERS, int16_t numbers[]) {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	// Then sort it
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		// uint16_t x=NUMNUMBERS-i-1; // Somehow this makes it a little slower at -Os. I expected no difference.
		for (uint16_t j=0; j<NUMNUMBERS-i-1; j++) {
			if (numbers[j]>numbers[j+1]) {
				int16_t temp = numbers[j];
				numbers[j] = numbers[j+1];
				numbers[j+1] = temp;
			}
		}
	}

	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCTestBubbleSort_void_test_bubblesort_native() {
	uint16_t NUMNUMBERS = 256;
	int16_t numbers[256];

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	javax_rtcbench_RTCTestBubbleSort_void_test_bubblesort_native2(NUMNUMBERS, numbers);
}
