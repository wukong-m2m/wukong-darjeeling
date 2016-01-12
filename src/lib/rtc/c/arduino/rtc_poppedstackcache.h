#ifndef RTC_POPPEDSTACKCACHE_H
#define RTC_POPPEDSTACKCACHE_H
#include <stdint.h>
#include "rtc.h"

void rtc_stackcache_init(rtc_translationstate *ts);

void rtc_stackcache_getfree_16bit(uint8_t *regs);
void rtc_stackcache_getfree_16bit_but_only_if_we_wont_spill(uint8_t *regs);
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


#define RTC_VALUETAG_TYPE_LOCAL     0x0000
#define RTC_VALUETAG_TYPE_STATIC    0x4000
#define RTC_VALUETAG_TYPE_CONSTANT  0x8000
#define RTC_VALUETAG_UNUSED         0xFFFF
#define RTC_VALUETAG_DATATYPE_REF   0x0000
#define RTC_VALUETAG_DATATYPE_SHORT 0x1000
#define RTC_VALUETAG_DATATYPE_INT   0x2000
#define RTC_VALUETAG_DATATYPE_INT_L 0x3000
#define RTC_VALUETAG_IS_REF(tag)    (((tag) & 0x3000) == RTC_VALUETAG_DATATYPE_REF)
#define RTC_VALUETAG_IS_SHORT(tag)  (((tag) & 0x3000) == RTC_VALUETAG_DATATYPE_SHORT)
#define RTC_VALUETAG_IS_INT(tag)    (((tag) & 0x3000) == RTC_VALUETAG_DATATYPE_INT)
#define RTC_VALUETAG_IS_INT_L(tag)  (((tag) & 0x3000) == RTC_VALUETAG_DATATYPE_INT_L)
#define RTC_VALUETAG_TO_INT_L(tag)  ((tag) + 0x1000)

bool rtc_poppedstackcache_can_I_skip_this();
uint16_t rtc_poppedstackcache_getvaluetag(uint8_t *regs);
void rtc_poppedstackcache_setvaluetag(uint8_t *regs, uint16_t valuetag);
void rtc_poppedstackcache_setvaluetag_int(uint8_t *regs, uint16_t valuetag);
void rtc_poppedstackcache_clearvaluetag(uint8_t reg_base);
void rtc_poppedstackcache_clear_all_valuetags();
void rtc_poppedstackcache_clear_all_with_valuetag(uint16_t valuetag);
void rtc_poppedstackcache_clear_all_callused_valuetags();
#endif // RTC_POPPEDSTACKCACHE_H