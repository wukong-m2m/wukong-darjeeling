#ifndef RTC_INSTRUCTIONS_H
#define RTC_INSTRUCTIONS_H

#include <stdint.h>
#include "rtc.h"

uint16_t rtc_translate_single_instruction(uint16_t pc, rtc_translationstate *translationstate);

#endif // RTC_INSTRUCTIONS_H
