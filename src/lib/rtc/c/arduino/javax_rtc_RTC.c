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

void javax_rtc_RTC_void_avroraPrintSP() {
    avroraPrintSP();
}
