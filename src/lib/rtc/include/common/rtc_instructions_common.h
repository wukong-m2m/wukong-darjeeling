#ifndef RTC_INSTRUCTIONS_COMMON_H
#define RTC_INSTRUCTIONS_COMMON_H

#include <stdint.h>

void rtc_common_translate_invoke(uint8_t opcode, uint8_t jvm_operand_byte0, uint8_t jvm_operand_byte1, uint8_t jvm_operand_byte2);

#endif // RTC_INSTRUCTIONS_COMMON_H