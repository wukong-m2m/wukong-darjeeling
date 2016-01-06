#ifndef RTC_STACKCACHE_H
#define RTC_STACKCACHE_H
#include <stdint.h>
#include "rtc.h"

void rtc_stackcache_init(rtc_translationstate *ts);

void rtc_stackcache_getfree_16bit(uint8_t *regs);
void rtc_stackcache_getfree_32bit(uint8_t *regs);
void rtc_stackcache_getfree_ref(uint8_t *regs);
bool rtc_stackcache_getfree_16bit_prefer_ge_R16(uint8_t *regs);

void rtc_stackcache_push_16bit(uint8_t *regs);
void rtc_stackcache_push_32bit(uint8_t *regs);
void rtc_stackcache_push_ref(uint8_t *regs);
void rtc_stackcache_push_16bit_from_scratch_R24R25();
void rtc_stackcache_push_32bit_from_scratch_R22R25();
void rtc_stackcache_push_ref_from_scratch_R24R25();

void rtc_stackcache_pop_16bit(uint8_t *regs);
void rtc_stackcache_pop_32bit(uint8_t *regs);
void rtc_stackcache_pop_ref(uint8_t *regs);
void rtc_stackcache_pop_16bit_into_fixed_reg(uint8_t reg_base);         // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
void rtc_stackcache_pop_32bit_into_fixed_reg(uint8_t reg_base);         // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
void rtc_stackcache_pop_ref_into_fixed_reg(uint8_t reg_base);           // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
void rtc_stackcache_pop_ref_into_Z();                                   // Pops a value into Z.

void rtc_stackcache_clear_call_used_regs_before_native_function_call(); // Pushes all call-used registers onto the stack, removing them from the cache (R18–R27, R30, R31)

void rtc_stackcache_flush_all_regs();                                   // Pushes all registers onto the stack, so the stack is in the state the next JVM method expects

void rtc_stackcache_next_instruction();

bool rtc_poppedstackcache_can_I_skip_this();
void rtc_poppedstackcache_brtarget();

#endif // RTC_STACKCACHE_H