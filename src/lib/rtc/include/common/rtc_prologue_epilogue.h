#include <stdint.h>

#define RTC_PROLOGUE_MAX_SIZE 42

void rtc_current_method_set_uses_reg(uint8_t reg);
bool rtc_current_method_get_uses_reg(uint8_t reg);
uint8_t rtc_current_method_prologue_size();
void rtc_emit_prologue();
void rtc_emit_epilogue();

