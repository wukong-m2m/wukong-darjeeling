#include <stdint.h>
#include "darjeeling3.h"
#include "debug.h"
#include "xxtea.h"

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

// Split into separate function to avoid the compiler just optimising away the whole test.
void __attribute__((noinline)) rtcbenchmark_measure_native_performance(uint32_t *v, uint8_t n, uint32_t const key[4]) {
	javax_darjeeling_Stopwatch_void_resetAndStart();

	uint32_t y, z, sum;
	uint8_t p, rounds, e; // Setting n and p to be 8 bit means we can't handle large arrays, but on a sensor node that should be fine)
	if (n > 1) {          /* Coding Part */
		rounds = 6 + 52/n;
		sum = 0;
		z = v[n-1];
		do {
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p=0; p<n-1; p++) {
				y = v[p+1]; 
				z = v[p] += MX;
			}
			y = v[0];
			z = v[n-1] += MX;
		} while (--rounds);
	} else if (n < -1) {  /* Decoding Part */
		n = -n;
		rounds = 6 + 52/n;
		sum = rounds*DELTA;
		y = v[0];
		do {
			e = (sum >> 2) & 3;
			for (p=n-1; p>0; p--) {
				z = v[p-1];
				y = v[p] -= MX;
			}
			z = v[n-1];
			y = v[0] -= MX;
			sum -= DELTA;
		} while (--rounds);
	}

	javax_darjeeling_Stopwatch_void_measure();
}

void javax_rtcbench_RTCBenchmark_void_test_native() {
	uint8_t NUMNUMBERS = 32;
	uint32_t numbers[NUMNUMBERS];
	uint32_t const key[4] = {0, 1, 2, 3};

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++)
		numbers[i] = (NUMNUMBERS - 1 - i);

	rtcbenchmark_measure_native_performance(numbers, NUMNUMBERS, key);

	for (uint16_t i=0; i<NUMNUMBERS; i++)
	    avroraPrintInt32((int32_t)numbers[i]);
	avroraPrintStr("done.");
}

