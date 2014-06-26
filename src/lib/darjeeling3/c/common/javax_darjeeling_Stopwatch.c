#include "djtimer.h"

dj_time_t stopwatch_start_time;

void javax_darjeeling_Stopwatch_void_resetAndStart() {
	stopwatch_start_time = dj_timer_getTimeMillis();
}

void javax_darjeeling_Stopwatch_void_measure() {
	DARJEELING_PRINTF("%c[31mSTOPWATCH: %ld ms %c[30m\n hallo", 0x1b, dj_timer_getTimeMillis() - stopwatch_start_time, 0x1b);
}
