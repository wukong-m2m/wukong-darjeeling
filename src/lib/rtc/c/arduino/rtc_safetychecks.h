#ifndef RTC_SAFETYCHECKS_H
#define RTC_SAFETYCHECKS_H

#include <stdint.h>

void rtc_safety_method_starts();
void rtc_safety_process_opcode(uint8_t opcode);
void rtc_safety_check_offset_valid_for_local_variable(uint16_t offset);
uint16_t rtc_safety_check_offset_valid_for_static_variable(dj_infusion *infusion_ptr, uint8_t size, volatile uint16_t offset);
void rtc_safety_method_ends();

void rtc_safety_abort_with_error(uint8_t error);

#define RTC_SAFETYCHECK_METHOD_SHOULD_END_IN_BRANCH_OR_RETURN          0
#define RTC_SAFETYCHECK_INT_STACK_UNDERFLOW                            1
#define RTC_SAFETYCHECK_REF_STACK_UNDERFLOW                            2
#define RTC_SAFETYCHECK_INT_STACK_OVERFLOW                             3
#define RTC_SAFETYCHECK_REF_STACK_OVERFLOW                             4
#define RTC_SAFETYCHECK_STACK_NOT_EMPTY_AFTER_RETURN                   5
#define RTC_SAFETYCHECK_STACK_NOT_EMPTY_AT_BRANCH                      6
#define RTC_SAFETYCHECK_STACK_NOT_EMPTY_AT_BRANCHTARGET                7
#define RTC_SAFETYCHECK_BRANCHTARGET_COUNT_MISMATCH_WITH_METHOD_HEADER 8
#define RTC_SAFETYCHECK_BRANCH_TO_NONEXISTANT_BRTARGET                 9
#define RTC_SAFETYCHECK_STORE_TO_NONEXISTANT_LOCAL_VARIABLE           10
#define RTC_SAFETYCHECK_STORE_TO_NONEXISTANT_STATIC_VARIABLE          11

#endif // RTC_SAFETYCHECKS_H
