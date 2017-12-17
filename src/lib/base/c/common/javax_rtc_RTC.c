#include "config.h"
#include "djtimer.h"
#include "execution.h"

void javax_rtc_RTC_void_useRTC_boolean() {
	dj_exec_use_rtc = dj_exec_stackPopShort();
}

void javax_rtc_RTC_void_avroraBreak() {
	asm volatile ("break");
}

void javax_rtc_RTC_void_avroraPrintShort_short() {
    int16_t value = dj_exec_stackPopShort();
    avroraPrintInt16(value);
}

void javax_rtc_RTC_void_avroraPrintInt_int() {
    int32_t value = dj_exec_stackPopInt();
    avroraPrintInt32(value);
}

void javax_rtc_RTC_void_avroraPrintHex16_short() {
    int32_t value = dj_exec_stackPopShort();
    avroraPrintHex16(value);
}

void javax_rtc_RTC_void_avroraPrintHex32_int() {
    int32_t value = dj_exec_stackPopInt();
    avroraPrintHex32(value);
}

void javax_rtc_RTC_void_avroraPrintSP() {
    avroraPrintSP();
}

void javax_rtc_RTC_void_avroraStartCountingCalls() {
    avroraRTCStartCountingCalls();
}

void javax_rtc_RTC_void_avroraStopCountingCalls() {
    avroraRTCStopCountingCalls();
}

void javax_rtc_RTC_void_beep_int() {
	uint8_t number = (uint8_t)dj_exec_stackPopInt();
	avroraRTCRuntimeBeep(number);
}

void javax_rtc_RTC_void_terminateOnException_short() {
    int16_t type = dj_exec_stackPopShort();
    avroraTerminateOnException(type);	
}

// =========== COREMARK STUFF ===========
dj_time_t CorePortMe_start;
dj_time_t CorePortMe_stop;


void javax_rtc_RTC_void_coremark_start_time_nat() {
    CorePortMe_start = dj_timer_getTimeMillis();
}

void javax_rtc_RTC_void_coremark_stop_time() {
    CorePortMe_stop = dj_timer_getTimeMillis();
}

void javax_rtc_RTC_int_coremark_get_time() {
    dj_exec_stackPushInt(CorePortMe_stop - CorePortMe_start);
}
