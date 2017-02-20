#include "config.h"
#include "djtimer.h"
#include "execution.h"

dj_time_t CorePortMe_start;
dj_time_t CorePortMe_stop;

void javax_rtcbench_CorePortMe_void_start_time() {
	CorePortMe_start = dj_timer_getTimeMillis();
}

void javax_rtcbench_CorePortMe_void_stop_time() {
	CorePortMe_stop = dj_timer_getTimeMillis();
}

void javax_rtcbench_CorePortMe_int_get_time() {
	dj_exec_stackPushInt(CorePortMe_stop - CorePortMe_start);
}
