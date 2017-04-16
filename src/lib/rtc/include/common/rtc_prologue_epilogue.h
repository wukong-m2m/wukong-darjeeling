#include <stdint.h>

#define RTC_PROLOGUE_MAX_SIZE 46

uint8_t rtc_current_method_prologue_size();
void rtc_emit_prologue();
void rtc_emit_epilogue();

