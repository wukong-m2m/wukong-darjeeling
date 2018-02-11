#include "execution.h"
#include "array.h"

extern const int16_t Sinewave[1024] PROGMEM; // Defined in fft.c

void javax_rtcbench_RTCBenchmark_void_initSinewave_short__() {
	dj_int_array* byteStr = REF_TO_VOIDP(dj_exec_stackPopRef());
	int16_t* jvm_sinewave = byteStr->data.shorts;

	for (uint16_t i=0; i<1024; i++) {
		jvm_sinewave[i] = pgm_read_word(&Sinewave[i]);
	}
}
