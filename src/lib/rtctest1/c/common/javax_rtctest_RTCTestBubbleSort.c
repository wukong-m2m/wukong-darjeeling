#include <stdint.h>
#include "darjeeling3.h"

// Split into separate function to avoid the compiler just optimising away the whole test.
void javax_rtctest_RTCTestBubbleSort_void_test_bubblesort_native2(uint16_t NUMNUMBERS, uint16_t numbers[]) {
	// Fill the array
	for (int i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	// Then sort it
	for (short i=0; i<NUMNUMBERS; i++) {
		for (short j=0; j<NUMNUMBERS-i-1; j++) {
			if (numbers[j]>numbers[j+1]) {
				short temp = numbers[j];
				numbers[j] = numbers[j+1];
				numbers[j+1] = temp;
			}
		}
	}
}

void javax_rtctest_RTCTestBubbleSort_void_test_bubblesort_native() {
	uint16_t NUMNUMBERS = 256;
	uint16_t numbers[256];

	javax_darjeeling_Stopwatch_void_resetAndStart();

	javax_rtctest_RTCTestBubbleSort_void_test_bubblesort_native2(NUMNUMBERS, numbers);

	javax_darjeeling_Stopwatch_void_measure();
}
