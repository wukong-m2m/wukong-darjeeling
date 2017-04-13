#ifdef AOT_STRATEGY_SIMPLESTACKCACHE

#include <stdint.h>
#include <stdbool.h>
#include "panic.h"
#include "rtc.h"
#include "rtc_simplestackcache.h"
#include "asm.h"

#define RTC_STACKCACHE_AVAILABLE                     0xFF
#define RTC_STACKCACHE_IN_USE                        0xFE
#define RTC_STACKCACHE_DISABLED                      0xFD

#define RTC_STACKCACHE_INT_STACK_TYPE                0x00
#define RTC_STACKCACHE_REF_STACK_TYPE                0x10
#define RTC_STACKCACHE_IS_AVAILABLE(idx)             (rtc_stackcache_state[(idx)] == RTC_STACKCACHE_AVAILABLE)
#define RTC_STACKCACHE_IS_IN_USE(idx)                (rtc_stackcache_state[(idx)] == RTC_STACKCACHE_IN_USE)
#define RTC_STACKCACHE_IS_DISABLED(idx)              (rtc_stackcache_state[(idx)] == RTC_STACKCACHE_DISABLED)
#define RTC_STACKCACHE_IS_ON_STACK(idx)              ((rtc_stackcache_state[(idx)] & 0x80) == 0)
#define RTC_STACKCACHE_STACK_DEPTH_FOR_IDX(idx)      (RTC_STACKCACHE_IS_ON_STACK(idx) ? rtc_stackcache_state[(idx)] & 0x0F : 0xFF)
#define RTC_STACKCACHE_IS_INT_STACK(idx)             ((rtc_stackcache_state[(idx)] & 0x10) == RTC_STACKCACHE_INT_STACK_TYPE)
#define RTC_STACKCACHE_IS_REF_STACK(idx)             ((rtc_stackcache_state[(idx)] & 0x10) == RTC_STACKCACHE_REF_STACK_TYPE)
#define RTC_STACKCACHE_IS_STACK_TYPE(idx, type)      ((rtc_stackcache_state[(idx)] & 0x10) == (type))

#define RTC_STACKCACHE_INC_DEPTH(idx)                (rtc_stackcache_state[(idx)]++)
#define RTC_STACKCACHE_DEC_DEPTH(idx)                (rtc_stackcache_state[(idx)]--)
#define RTC_STACKCACHE_MARK_AVAILABLE(idx)           (rtc_stackcache_state[(idx)] = RTC_STACKCACHE_AVAILABLE)
#define RTC_STACKCACHE_MARK_IN_USE(idx)              (rtc_stackcache_state[(idx)] = RTC_STACKCACHE_IN_USE)
#define RTC_STACKCACHE_MARK_DISABLED(idx)            (rtc_stackcache_state[(idx)] = RTC_STACKCACHE_DISABLED)
#define RTC_STACKCACHE_MARK_INT_STACK_DEPTH0(idx)    (rtc_stackcache_state[(idx)] = RTC_STACKCACHE_INT_STACK_TYPE)
#define RTC_STACKCACHE_MARK_REF_STACK_DEPTH0(idx)    (rtc_stackcache_state[(idx)] = RTC_STACKCACHE_REF_STACK_TYPE)
#define RTC_STACKCACHE_MOVE_CACHE_ELEMENT(dest, src) {rtc_stackcache_state[(dest)] = rtc_stackcache_state[(src)]; RTC_STACKCACHE_MARK_IN_USE(src); }
uint8_t rtc_stackcache_state[RTC_STACKCACHE_MAX_IDX];

// IMPORTANT: REGISTERS ARE ALWAYS ASSIGNED IN PAIRS:
// AFTER getfreereg/pop_16bit(regs)
//     regs[0]+1 == regs[1]
// AND
// AFTER getfreereg/pop_32bit(regs)
//     regs[0]+1 == regs[1]
//     regs[2]+1 == regs[3]
//     but regs[0:1] and regs[2:3] aren't guaranteed to be consequetive

// Register states:
// Only stored for even numbered registers (for the pair x:x+1)
// AVAILABLE: 0xFF
// IN USE   : 0xFE
// ON STACK : 000rdddd, with
//            r    = 1 for reference stack element, 0 for integer stack element
//            dddd = stack depth. the top element has d=0, then next d=1, etc.
//
// A pop always pops the register with the highest d, but because of
// the way Darjeeling split the reference and integer stack, there may
// be an element with higher d on the other stack.
//
// state transitions:
// AVAILABLE -> IN USE              : getfree
// IN USE -> ON STACK               : push (if source not yet on stack)
// ON STACK -> IN USE               : pop
// IN USE -> AVAILABLE              : next_instruction
// AVAILABLE -> ON STACK            : push (if source already on stack)
//
// ON STACK -> AVAILABLE            : pop_into_fixed (for reg containing value)
// AVAILABLE -> IN USE              : pop_into_fixed (for target reg)

// Calling convention:
// Call used:  r0, r18-r25, r26:r27 (X), r30:r31 (Z)
// Call saved: r1, r2-r17, r28:r29 (Y)
// In use:
// r0:r1   : fixed registers
// r2:r3   : pointer to infusion's static fields
// X       : ref stack pointer
// Y       : JVM frame pointer
// Z       : scratch/pointer to infusion's static fields or to an object (need to load at each operation)



// Private utility functions
void rtc_stackcache_assert_no_in_use() {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_IN_USE(idx)) {
            dj_panic(DJ_PANIC_AOT_STACKCACHE_IN_USE);
        }
    }
}
#define RTC_NUMBER_OF_CALL_USED_REGS_PAIRS 3
bool rtc_stackcache_is_call_used_idx(uint8_t idx) {
    uint8_t reg = ARRAY_INDEX_TO_REG(idx);
    return reg == R18 || reg == R20 || reg == R22 || reg == R24;
}
uint8_t get_deepest_pair_idx() { // Returns 0xFF if there's nothing on the stack.
    int8_t deepest_depth = -1;
    uint8_t deepest_idx = 0xFF;
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_ON_STACK(idx) && RTC_STACKCACHE_STACK_DEPTH_FOR_IDX(idx) > deepest_depth) {
            deepest_idx = idx;
            deepest_depth = RTC_STACKCACHE_STACK_DEPTH_FOR_IDX(idx);
        }
    }
    return deepest_idx;
}
uint8_t rtc_stackcache_spill_deepest_pair() { // Returns the idx of the freed slot
    // Registers ON STACK are numbered consecutively from 0 up.
    // Find the highest/deepest element and spill it to the real stack.
    // Post: the ON STACK register with the highest number is pushed to the real stack, and changed to AVAILABLE
    uint8_t idx = get_deepest_pair_idx();
    if (idx != 0xFF) {
        // Found the deepest register that's on the stack. Push it to real memory, and mark it AVAILABLE
        if (RTC_STACKCACHE_IS_INT_STACK(idx)) {
            emit_x_PUSH_16bit(ARRAY_INDEX_TO_REG(idx));            
        } else {
            emit_x_PUSH_REF(ARRAY_INDEX_TO_REG(idx));            
        }
        RTC_STACKCACHE_MARK_AVAILABLE(idx);
        return idx;
    } else {
        while(true) { dj_panic(DJ_PANIC_AOT_STACKCACHE_NOTHING_TO_SPILL); }
    }
}
uint8_t rtc_get_stack_idx_at_depth(uint8_t depth) {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_STACK_DEPTH_FOR_IDX(idx) == depth) {
            return idx;
        }
    }
    return 0xFF;
}
uint8_t rtc_get_first_available_index() {
    for (int8_t idx=RTC_STACKCACHE_MAX_IDX-1; idx>=0; idx--) {
        if (RTC_STACKCACHE_IS_AVAILABLE(idx)) {
            return idx;
        }
    }
    return 0xFF;
}


// Public API
#define RTC_NUMBER_OF_USABLE_REGS_PAIRS 11
// #define this so we can control it from Gradle if we want. If not, default to using all available regs.
#ifndef RTC_STACKCACHE_NUMBER_OF_CACHE_REG_PAIRS_TO_USE
#define RTC_STACKCACHE_NUMBER_OF_CACHE_REG_PAIRS_TO_USE RTC_NUMBER_OF_USABLE_REGS_PAIRS
#endif
void rtc_stackcache_init(bool is_lightweight) {
    // First mark all regs as DISABLED.
    for (uint8_t i=0; i<RTC_STACKCACHE_MAX_IDX; i++) {
        RTC_STACKCACHE_MARK_DISABLED(i);
    }

    // // These are the registers we may use
    // uint8_t registers_we_can_use[RTC_NUMBER_OF_USABLE_REGS_PAIRS] = {
    //         R4, R6, R8, R10, R12, R14, R16, // Call saved
    //         R18, R20, R22, R24 // Call used
    // };

    // Depending on the defined number of actual registers to use, mark those AVAILABLE
    for (uint8_t i=0; i<RTC_STACKCACHE_NUMBER_OF_CACHE_REG_PAIRS_TO_USE; i++) {
        // uint8_t reg = registers_we_can_use[i];
        RTC_STACKCACHE_MARK_AVAILABLE(REG_TO_ARRAY_INDEX(R4+2*i));
    }
    if (is_lightweight) {
        // For lightweight methods we use R18:R19 to pop the return address into (not that this only works for <=128K devices with 2 byte PC)
        RTC_STACKCACHE_MARK_DISABLED(REG_TO_ARRAY_INDEX(R18));
    }
}

uint8_t rtc_stackcache_getfree_pair() {
    // If there's an available register, use it
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_AVAILABLE(idx)) {
            RTC_STACKCACHE_MARK_IN_USE(idx);
            return ARRAY_INDEX_TO_REG(idx);
        }
    }
    uint8_t freed_idx = rtc_stackcache_spill_deepest_pair();
    RTC_STACKCACHE_MARK_IN_USE(freed_idx);
    return ARRAY_INDEX_TO_REG(freed_idx);
}
void rtc_stackcache_getfree_16bit(uint8_t *regs) {
    uint8_t r = rtc_stackcache_getfree_pair();
    regs[0] = r;
    regs[1] = r+1;
}
void rtc_stackcache_getfree_32bit(uint8_t *regs) {
    uint8_t r = rtc_stackcache_getfree_pair();
    regs[0] = r;
    regs[1] = r+1;
    r = rtc_stackcache_getfree_pair();
    regs[2] = r;
    regs[3] = r+1;
}
void rtc_stackcache_getfree_ref(uint8_t *regs) {
    uint8_t r = rtc_stackcache_getfree_pair();
    regs[0] = r;
    regs[1] = r+1;
}
// Returns true if a register in the range >=r16 is allocated, false otherwise
bool rtc_stackcache_getfree_16bit_prefer_ge_R16(uint8_t *regs) {
    // Check if any reg in the range starting at R16 is free
    for(uint8_t reg=24; reg>=16; reg-=2) {
        uint8_t idx = REG_TO_ARRAY_INDEX(reg);
        if (RTC_STACKCACHE_IS_AVAILABLE(idx)) {
            // We're in luck.
            RTC_STACKCACHE_MARK_IN_USE(idx);
            regs[0] = reg;
            regs[1] = reg+1;
            return true;
        }
    }
    // No register available >= R16, get a free register using the normal way.
    rtc_stackcache_getfree_16bit(regs);
    // It's possible the previous function spilled a register that happens to
    // be >= R16, so we should check for that.
    return regs[0] >= R16;
}

void rtc_stackcache_push_pair(uint8_t reg_base, uint8_t which_stack) {
    uint8_t idx = REG_TO_ARRAY_INDEX(reg_base);
    if (RTC_STACKCACHE_IS_IN_USE(idx) || reg_base == R22 || reg_base == R24) {
        // shift depth for all pairs on the stack
        for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
            if (RTC_STACKCACHE_IS_ON_STACK(idx)) {
                RTC_STACKCACHE_INC_DEPTH(idx);
            }
        }
        // push this on the stack at depth 0
        if (which_stack == RTC_STACKCACHE_INT_STACK_TYPE) {
            RTC_STACKCACHE_MARK_INT_STACK_DEPTH0(idx);
        } else {
            RTC_STACKCACHE_MARK_REF_STACK_DEPTH0(idx);
        }
    } else {
        dj_panic(DJ_PANIC_AOT_STACKCACHE_PUSHED_REG_NOT_IN_USE);
    }
}
void rtc_stackcache_push_16bit_from_R22R23() {
    rtc_stackcache_push_pair(R22, RTC_STACKCACHE_INT_STACK_TYPE);
}
void rtc_stackcache_push_16bit_from_R24R25() {
    rtc_stackcache_push_pair(R24, RTC_STACKCACHE_INT_STACK_TYPE);
}
void rtc_stackcache_push_32bit_from_R22R25() {
    rtc_stackcache_push_pair(R24, RTC_STACKCACHE_INT_STACK_TYPE);
    rtc_stackcache_push_pair(R22, RTC_STACKCACHE_INT_STACK_TYPE);
}
void rtc_stackcache_push_ref_from_R22R23() {
    rtc_stackcache_push_pair(R22, RTC_STACKCACHE_REF_STACK_TYPE);
}
void rtc_stackcache_push_ref_from_R24R25() {
    rtc_stackcache_push_pair(R24, RTC_STACKCACHE_REF_STACK_TYPE);
}
// LET OP: ALS EEN GEPUSHT REGISTER AL OP DE STACK STAAT MOET HET WORDEN GEDUPLICEERD
void rtc_stackcache_push_16bit(uint8_t *regs) {
    rtc_stackcache_push_pair(regs[0], RTC_STACKCACHE_INT_STACK_TYPE);
}
void rtc_stackcache_push_32bit(uint8_t *regs) {
    rtc_stackcache_push_pair(regs[2], RTC_STACKCACHE_INT_STACK_TYPE);
    rtc_stackcache_push_pair(regs[0], RTC_STACKCACHE_INT_STACK_TYPE);
}
void rtc_stackcache_push_ref(uint8_t *regs) {
    rtc_stackcache_push_pair(regs[0], RTC_STACKCACHE_REF_STACK_TYPE);
}

uint8_t rtc_stackcache_pop_pair(uint8_t which_stack, uint8_t target_reg) {
    if (target_reg != 0xFF
     && target_reg != R18
     && target_reg != R20
     && target_reg != R22
     && target_reg != R24
     && target_reg != R26
     && target_reg != RZ) {
        while (true) {
            avroraPrintUInt8(which_stack);
            avroraPrintUInt8(target_reg);
         dj_panic(DJ_PANIC_AOT_STACKCACHE_INVALID_POP_TARGET); }
    }
    uint8_t target_idx = (target_reg == 0xFF)
                         ? 0xFF
                         : REG_TO_ARRAY_INDEX(target_reg);

    // The top element may be of the wrong stack type. This happens because of the way
    // Darjeeling transforms the single JVM stack into separate int and reference stacks.
    uint8_t depth = 0;
    while(true) {
        uint8_t stack_top_idx = rtc_get_stack_idx_at_depth(depth);
        if (stack_top_idx == 0xFF) {
            // Nothing found in the cache at this depth:
            // the value was spilled to the real stack at some point.
            // Get it back from memory and pop into either the target
            // register, or the first available register.
            if (target_idx == 0xFF) {
                target_idx = rtc_get_first_available_index();
                if (target_idx == 0xFF) {
                    while (true) { dj_panic(DJ_PANIC_AOT_STACKCACHE_NO_SPACE_FOR_POP); } // There should be enough space if there's nothing in the stack cache
                }
            }

            if (which_stack == RTC_STACKCACHE_INT_STACK_TYPE) {
                emit_x_POP_16bit(ARRAY_INDEX_TO_REG(target_idx));
            } else {
                emit_x_POP_REF(ARRAY_INDEX_TO_REG(target_idx));
            }
        } else {
            if (!RTC_STACKCACHE_IS_STACK_TYPE(stack_top_idx, which_stack)) {
                // The element at this stack depth is of the wrong type
                // Try the next one.
                depth++;
                continue;
            }

            // Element found.
            if (target_idx == 0xFF) { // If no specific target was specified, leave it here
                target_idx = stack_top_idx;
            }

            // Pop this element. Depths of deeper elements should decrease by 1
            for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
                if (RTC_STACKCACHE_IS_ON_STACK(idx) && RTC_STACKCACHE_STACK_DEPTH_FOR_IDX(idx) > depth) {
                    RTC_STACKCACHE_DEC_DEPTH(idx);
                }
            }

            // Check if we need to move the element to a specific target            
            if (target_idx != stack_top_idx) {
                // The value needs to go to a specific target, which is different from where it is now.
                emit_MOVW(target_reg, ARRAY_INDEX_TO_REG(stack_top_idx)); // Move the value to the target register
                RTC_STACKCACHE_MARK_AVAILABLE(stack_top_idx); // Original location is now AVAILABLE
            }
        }
        if (target_idx != REG_TO_ARRAY_INDEX(RX) && target_idx != REG_TO_ARRAY_INDEX(RZ)) { // Don't mark X or Z in use, because it should't become available after this instruction
            RTC_STACKCACHE_MARK_IN_USE(target_idx); // Target is now IN USE (if it wasn't already)
        }
        return ARRAY_INDEX_TO_REG(target_idx);

    }
}
void rtc_stackcache_pop_16bit(uint8_t *regs) {
    uint8_t r = rtc_stackcache_pop_pair(RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
    regs[0] = r;
    regs[1] = r+1;
}
void rtc_stackcache_pop_32bit(uint8_t *regs) {
    uint8_t r = rtc_stackcache_pop_pair(RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
    regs[0] = r;
    regs[1] = r+1;
    r = rtc_stackcache_pop_pair(RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
    regs[2] = r;
    regs[3] = r+1;
}
void rtc_stackcache_pop_ref(uint8_t *regs) {
    uint8_t r = rtc_stackcache_pop_pair(RTC_STACKCACHE_REF_STACK_TYPE, 0xFF);
    regs[0] = r;
    regs[1] = r+1;
}
// Pops a value into a specific range of consecutive regs. Panics if any reg is in use.
void rtc_stackcache_pop_16bit_into_fixed_reg(uint8_t reg_base) {
    rtc_stackcache_pop_pair(RTC_STACKCACHE_INT_STACK_TYPE, reg_base);
}
void rtc_stackcache_pop_32bit_into_fixed_reg(uint8_t reg_base) {
    rtc_stackcache_pop_pair(RTC_STACKCACHE_INT_STACK_TYPE, reg_base);
    rtc_stackcache_pop_pair(RTC_STACKCACHE_INT_STACK_TYPE, reg_base+2);
}
void rtc_stackcache_pop_ref_into_fixed_reg(uint8_t reg_base) {
    rtc_stackcache_pop_pair(RTC_STACKCACHE_REF_STACK_TYPE, reg_base);
}
void rtc_stackcache_pop_ref_into_Z() {
    rtc_stackcache_pop_pair(RTC_STACKCACHE_REF_STACK_TYPE, RZ);
}

bool rtc_stackcache_has_ref_in_cache() {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_ON_STACK(idx) && RTC_STACKCACHE_IS_REF_STACK(idx)) {
            return true;
        }
    }
    return false;
}
void rtc_stackcache_clear_call_used_regs_and_refs_before_native_function_call() {
    // Pushes all call-used registers onto the stack, removing them from the cache (R18–R25)
    rtc_stackcache_assert_no_in_use();

    // After this all call-used registers are marked IN USE, so they can be pushed on the stack
    // to store a function result
    while(true) {
        // Check if there's any call-used reg on the stack
        uint8_t call_used_reg_on_stack_idx = 0xFF;
        uint8_t call_saved_reg_available_idx = 0xFF;
        for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
            // Is this an available call-saved register?
            if (!rtc_stackcache_is_call_used_idx(idx) && RTC_STACKCACHE_IS_AVAILABLE(idx)) {
                call_saved_reg_available_idx = idx;
            }

            // Is this an on-stack call-used register?
            if (rtc_stackcache_is_call_used_idx(idx)) {
                if (RTC_STACKCACHE_IS_ON_STACK(idx)) { // It's on the stack: we need to move it to memory
                    call_used_reg_on_stack_idx = idx;
                }
                if (RTC_STACKCACHE_IS_AVAILABLE(idx)) { // It's available: mark it IN USE
                    RTC_STACKCACHE_MARK_IN_USE(idx);    
                }
            }
        }

        if (call_used_reg_on_stack_idx == 0xFF) {
            break; // No call-used regs are on the stack, so we're almost done.
        }

        // There's a call used register on the stack.
        if (call_saved_reg_available_idx == 0xFF) {
            // No call-saved registers available, so we need to free one
            rtc_stackcache_spill_deepest_pair();
            // Either this was a call-used register, in which case we may be done now,
            // or this was a call-saved register, in which case we can use it in the
            // next iteration.
            continue;
        } else {
            // There's also an available calls-saved register, so move the value in the call-used reg there.
            emit_MOVW(ARRAY_INDEX_TO_REG(call_saved_reg_available_idx), ARRAY_INDEX_TO_REG(call_used_reg_on_stack_idx));
            // Update the cache state
            RTC_STACKCACHE_MOVE_CACHE_ELEMENT(call_saved_reg_available_idx, call_used_reg_on_stack_idx);
        }
    }
    // Also spill any references in call-saved registers since they may not be valid anymore if the garbage collector runs
    while (rtc_stackcache_has_ref_in_cache()) {
        rtc_stackcache_spill_deepest_pair();
    }
}

void rtc_stackcache_flush_all_regs() {
    while (get_deepest_pair_idx() != 0xFF) {
        rtc_stackcache_spill_deepest_pair();
    }
}

void rtc_stackcache_next_instruction() {
    avroraRTCTraceStackCacheState(rtc_stackcache_state); // Store it here so we can see what's IN USE
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_IN_USE(idx)) {
            RTC_STACKCACHE_MARK_AVAILABLE(idx);
        }
    }
}

#endif // AOT_STRATEGY_SIMPLESTACKCACHE
