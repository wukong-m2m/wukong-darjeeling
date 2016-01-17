#ifndef RTC_MARKLOOP_H
#define RTC_MARKLOOP_H
#include <stdint.h>
#include "rtc.h"

#define RTC_VALUETAG_TO_INT_L(tag)                  ((tag) + 0x1000)

void rtc_stackcache_init(rtc_translationstate *ts);
void rtc_stackcache_next_instruction();

// GETFREE
//    finds a free register pair, possibly spilling the lowest pair to memory to free it up
//    clears the valuetag of the register pair returned
void rtc_stackcache_getfree_16bit(uint8_t *regs);
void rtc_stackcache_getfree_32bit(uint8_t *regs);
void rtc_stackcache_getfree_ref(uint8_t *regs);
void rtc_stackcache_getfree_16bit_but_only_if_we_wont_spill_otherwise_clear_valuetag(uint8_t *regs);
bool rtc_stackcache_getfree_16bit_prefer_ge_R16(uint8_t *regs);

// PUSH
//    if we're pushing from R24, copy it to a free register first (calls GETFREE internally)
//    if the valuetag is set for the current instruction, record it
void rtc_stackcache_push_16bit(uint8_t *regs);
void rtc_stackcache_push_32bit(uint8_t *regs);
void rtc_stackcache_push_ref(uint8_t *regs);
void rtc_stackcache_push_16bit_from_R24R25();
void rtc_stackcache_push_32bit_from_R22R25();
void rtc_stackcache_push_ref_from_R24R25();

// POP
//    if nondestructive: pop, retaining the valuetag so it may be recycled later
//    if destructive: pop, and clear the valuetag since the calling instruction will destroy the value
//    if to_store: pop, and set the valuetag since we will write this value to memory. also clear any other register with the same value tag since it's no longer current
void rtc_stackcache_pop_nondestructive_16bit(uint8_t *regs);
void rtc_stackcache_pop_nondestructive_32bit(uint8_t *regs);
void rtc_stackcache_pop_nondestructive_ref(uint8_t *regs);
void rtc_stackcache_pop_destructive_16bit(uint8_t *regs);
void rtc_stackcache_pop_destructive_32bit(uint8_t *regs);
void rtc_stackcache_pop_destructive_ref(uint8_t *regs);
void rtc_stackcache_pop_to_store_16bit(uint8_t *regs);
void rtc_stackcache_pop_to_store_32bit(uint8_t *regs);
void rtc_stackcache_pop_to_store_ref(uint8_t *regs);

void rtc_stackcache_pop_destructive_16bit_into_fixed_reg(uint8_t reg_base);         // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
void rtc_stackcache_pop_destructive_32bit_into_fixed_reg(uint8_t reg_base);         // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
void rtc_stackcache_pop_destructive_ref_into_fixed_reg(uint8_t reg_base);           // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
void rtc_stackcache_pop_destructive_ref_into_Z();                                   // Pops a value into Z. Panics if any reg is not marked IN USE.

void rtc_stackcache_flush_call_used_regs_and_clear_valuetags();         // Pushes all call-used registers onto the stack, removing them from the cache (R18–R27, R30, R31), and clears the value tags
void rtc_stackcache_flush_all_regs();                                   // Pushes all registers onto the stack, so the stack is in the state the next JVM method expects, but preserves the value tags

uint16_t rtc_poppedstackcache_get_valuetag(uint8_t *regs);
void rtc_poppedstackcache_set_valuetag(uint8_t *regs, uint16_t valuetag);
void rtc_poppedstackcache_clear_all_except_pinned_with_valuetag(uint16_t valuetag);
void rtc_poppedstackcache_clear_all_callused_valuetags();
void rtc_poppedstackcache_clear_all_except_pinned_valuetags();

bool rtc_poppedstackcache_can_I_skip_this();
void rtc_markloop_emit_prologue();
void rtc_markloop_emit_epilogue();

// emit instructions to be used by MARKLOOP prologue and epilogue as well as normal instructions
void emit_load_local_16bit(uint8_t *regs, uint16_t offset);
void emit_load_local_32bit(uint8_t *regs, uint16_t offset);
#define emit_load_local_ref(regs, offset) emit_load_local_16bit(regs, offset)

void emit_store_local_16bit(uint8_t *regs, uint16_t offset);
void emit_store_local_32bit(uint8_t *regs, uint16_t offset);
#define emit_store_local_ref(regs, offset) emit_store_local_16bit(regs, offset)

#endif // RTC_MARKLOOP_H