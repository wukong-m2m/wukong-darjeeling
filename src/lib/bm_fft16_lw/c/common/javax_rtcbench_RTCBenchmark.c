#include <stdint.h>
#include "config.h"
#include "darjeeling3.h"
#include "fft.h"

#define RTCTEST_FFT_ARRAYSIZE 6

void javax_rtcbench_RTCBenchmark_void_test_native() {
	const uint16_t NUMNUMBERS = 1<<RTCTEST_FFT_ARRAYSIZE;
	int16_t data[NUMNUMBERS];
	int16_t im[NUMNUMBERS];

	// Fill the array
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		data[i] = i*16;
		im[i] = 0;
	}

	avroraPrintStr("BEFORE FFT");
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		avroraPrintStr("-----");
		avroraPrintInt16(data[i]);
		avroraPrintInt16(im[i]);
	}

	rtcbenchmark_measure_native_performance(data, im, RTCTEST_FFT_ARRAYSIZE, 0);

	avroraPrintStr("AFTER FFT");
	for (uint16_t i=0; i<NUMNUMBERS; i++) {
		avroraPrintStr("-----");
		avroraPrintInt16(data[i]);
		avroraPrintInt16(im[i]);
	}
}

