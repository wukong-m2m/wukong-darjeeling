#ifndef RTC_SAFETYCHECKS_H
#define RTC_SAFETYCHECKS_H

#include <stdint.h>

void rtc_safety_process_opcode(uint8_t opcode);
void rtc_safety_method_ends();

#define RTC_SAFETYCHECK_METHOD_SHOULD_END_IN_BRANCH_OR_RETURN 0
#define RTC_SAFETYCHECK_INT_STACK_UNDERFLOW                   1
#define RTC_SAFETYCHECK_REF_STACK_UNDERFLOW                   2
#define RTC_SAFETYCHECK_INT_STACK_OVERFLOW                    3
#define RTC_SAFETYCHECK_REF_STACK_OVERFLOW                    4
#define RTC_SAFETYCHECK_STACK_NOT_EMPTY_AFTER_RETURN          5
#define RTC_SAFETYCHECK_STACK_NOT_EMPTY_AT_BRANCH             6
#define RTC_SAFETYCHECK_STACK_NOT_EMPTY_AT_BRANCHTARGET       7

#endif // RTC_SAFETYCHECKS_H
