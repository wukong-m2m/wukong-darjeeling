#ifndef RTC_SAFETYCHECKS_H
#define RTC_SAFETYCHECKS_H

#include <stdint.h>
#include "rtc_safetychecks_vm_part.h"

// Compile time
void rtc_safety_method_starts();
void rtc_safety_process_opcode(uint8_t opcode);
void rtc_safety_check_offset_valid_for_local_variable(uint16_t offset);
uint16_t rtc_safety_check_offset_valid_for_static_variable(dj_infusion *infusion_ptr, uint8_t size, volatile uint16_t offset);
void rtc_safety_method_ends();

// Run time
void rtc_safety_mem_check();

#endif // RTC_SAFETYCHECKS_H
