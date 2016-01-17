#ifdef AOT_STRATEGY_POPPEDSTACKCACHE

#include <stdint.h>
#include <stdbool.h>
#include "panic.h"
#include "rtc_poppedstackcache.h"
#include "asm.h"
#include "rtc.h"
#include "opcodes.h"

#define RTC_STACKCACHE_MAX_IDX             16 // 16 because we only keep track of pairs
#define REG_TO_ARRAY_INDEX(reg)            ((reg)/2)
#define ARRAY_INDEX_TO_REG(idx)            ((idx)*2)

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
#define RTC_STACKCACHE_MOVE_CACHE_ELEMENT(dest, src) {rtc_stackcache_state[(dest)] = rtc_stackcache_state[(src)]; \
                                                      rtc_stackcache_valuetags[(dest)] = rtc_stackcache_valuetags[(src)]; \
                                                      RTC_STACKCACHE_MARK_IN_USE(src); }

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
#define RTC_STACKCACHE_SET_VALUETAG(idx, tag)       (rtc_stackcache_valuetags[(idx)] = tag)
#define RTC_STACKCACHE_GET_VALUETAG(idx)            (rtc_stackcache_valuetags[(idx)])
#define RTC_STACKCACHE_CLEAR_VALUETAG(idx)          (rtc_stackcache_valuetags[(idx)] = 0xFFFF)
#define RTC_STACKCACHE_UPDATE_AGE(idx)               (rtc_stackcache_age[(idx)] = rtc_ts->pc)
#define RTC_STACKCACHE_GET_AGE(idx)                  (rtc_stackcache_age[(idx)])

rtc_translationstate *rtc_ts; // Store a global pointer to the translation state. Bit of a hack, but this way I don't need to pass the pointer on each call.
uint8_t rtc_stackcache_state[RTC_STACKCACHE_MAX_IDX];
uint16_t rtc_stackcache_valuetags[RTC_STACKCACHE_MAX_IDX];
uint16_t rtc_stackcache_age[RTC_STACKCACHE_MAX_IDX];

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
// ON STACK -> AVAILABLE            : pop_into_fixed (for reg containing value)
// AVAILABLE -> IN USE              : pop_into_fixed (for target reg)
// IN USE -> AVAILABLE              : next_instruction
//
// Calling convention:
// Call used:  r0, r18-r25, r26:r27 (X), r30:r31 (Z)
// Call saved: r1, r2-r17, r28:r29 (Y)
// In use:
// r0:r1   : fixed registers
// r2:r3   : pointer to infusion's static fields
// X       : ref stack pointer
// Y       : JVM frame pointer
// Z       : scratch/pointer to infusion's static fields or to an object (need to load at each operation)


////////////////////// HELPERS
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
uint8_t rtc_get_lru_available_index() {
    uint16_t oldest_available_age = 0xFFFF;
    uint8_t idx_to_return = 0xFF;

    // We prefer registers without a value tag, since we can't recycle those anyway. (so it's technically not lru_available...)
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_AVAILABLE(idx) && RTC_STACKCACHE_GET_VALUETAG(idx)==RTC_VALUETAG_UNUSED) {
            idx_to_return = idx;
            break;
        }
    }

    // If there are no available registers without valuetag, we want to use the oldest one
    if (idx_to_return == 0xFF) {
        for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
            if (RTC_STACKCACHE_IS_AVAILABLE(idx) && RTC_STACKCACHE_GET_AGE(idx)<oldest_available_age) {
                oldest_available_age = RTC_STACKCACHE_GET_AGE(idx);
                idx_to_return = idx;
            }
        }
    }

    if (idx_to_return != 0xFF) {
        // Since this register will be used by whoever called rtc_get_lru_available_index, mark it IN USE and clear the valuetag because the value currently stored there will be overwritten.
        RTC_STACKCACHE_MARK_IN_USE(idx_to_return);
        RTC_STACKCACHE_CLEAR_VALUETAG(idx_to_return);        
    }

    // Return the oldest available register with a value tag, or this will be 0xFF if no register was available at all.
    return idx_to_return;
}

////////////////////// PUBLIC INTERFACE
#define RTC_NUMBER_OF_USABLE_REGS_PAIRS 11
// #define this so we can control it from Gradle if we want. If not, default to using all available regs.
#ifndef RTC_STACKCACHE_NUMBER_OF_CACHE_REG_PAIRS_TO_USE
#define RTC_STACKCACHE_NUMBER_OF_CACHE_REG_PAIRS_TO_USE RTC_NUMBER_OF_USABLE_REGS_PAIRS
#endif
void rtc_stackcache_init(rtc_translationstate *ts) {
    rtc_ts = ts;

    // First mark all regs as DISABLED.
    for (uint8_t i=0; i<RTC_STACKCACHE_MAX_IDX; i++) {
        RTC_STACKCACHE_MARK_DISABLED(i);
        RTC_STACKCACHE_SET_VALUETAG(i, RTC_VALUETAG_UNUSED);
        RTC_STACKCACHE_UPDATE_AGE(i);
    }

    // These are the registers we may use
    uint8_t registers_we_can_use[RTC_NUMBER_OF_USABLE_REGS_PAIRS] = {
            R4, R6, R8, R10, R12, R14, R16, // Call saved
            R18, R20, R22, R24 // Call used
    };

    // Depending on the defined number of actual registers to use, mark those AVAILABLE
    for (uint8_t i=0; i<RTC_STACKCACHE_NUMBER_OF_CACHE_REG_PAIRS_TO_USE; i++) {
        uint8_t reg = registers_we_can_use[i];
        RTC_STACKCACHE_MARK_AVAILABLE(REG_TO_ARRAY_INDEX(reg));
    }
}

void rtc_stackcache_next_instruction() {
    avroraRTCTraceStackCacheState(rtc_stackcache_state); // Store it here so we can see what's IN USE
    avroraRTCTraceStackCacheValuetags(rtc_stackcache_valuetags);
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_IN_USE(idx)) {
            RTC_STACKCACHE_MARK_AVAILABLE(idx);
        }
    }
}

// VALUETAG functions
uint16_t rtc_poppedstackcache_get_valuetag(uint8_t *regs) {
    return RTC_STACKCACHE_GET_VALUETAG(REG_TO_ARRAY_INDEX(regs[0]));
}
void rtc_poppedstackcache_set_valuetag(uint8_t *regs, uint16_t valuetag) {
    RTC_STACKCACHE_SET_VALUETAG(REG_TO_ARRAY_INDEX(regs[0]), valuetag);
}
void rtc_poppedstackcache_clear_all_with_valuetag(uint16_t valuetag) {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_GET_VALUETAG(idx) == valuetag) {
            RTC_STACKCACHE_CLEAR_VALUETAG(idx);
        }
    }
}
void rtc_poppedstackcache_clear_all_callused_valuetags() {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        // Is this a call-used register?
        if (rtc_stackcache_is_call_used_idx(idx)) {
            RTC_STACKCACHE_SET_VALUETAG(idx, RTC_VALUETAG_UNUSED);
        }
    }
}
void rtc_poppedstackcache_clear_all_valuetags() {
    // At a branch target we can't make any assumption about the register state since it will depend on the execution path. Clear all ages and valuetags to start with a clean cache.
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        RTC_STACKCACHE_CLEAR_VALUETAG(idx);
    }
}

// GETFREE
//    finds a free register pair, possibly spilling the lowest pair to memory to free it up
//    clears the valuetag of the register pair returned
uint8_t rtc_stackcache_getfree_pair() {
    // If there's an available register, use it
    uint8_t idx = rtc_get_lru_available_index();
    if (idx == 0xFF) {
        idx = rtc_stackcache_spill_deepest_pair();
    }
    RTC_STACKCACHE_MARK_IN_USE(idx);
    RTC_STACKCACHE_CLEAR_VALUETAG(idx);
    return ARRAY_INDEX_TO_REG(idx);
}
uint8_t rtc_stackcache_getfree_pair_but_only_if_we_wont_spill() {
    uint8_t idx = rtc_get_lru_available_index();
    if (idx != 0xFF) {
        RTC_STACKCACHE_MARK_IN_USE(idx);
        RTC_STACKCACHE_CLEAR_VALUETAG(idx);
        return ARRAY_INDEX_TO_REG(idx);
    }
    return 0xFF;
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
void rtc_stackcache_getfree_16bit_but_only_if_we_wont_spill_otherwise_clear_valuetag(uint8_t *regs) {
    uint8_t r = rtc_stackcache_getfree_pair_but_only_if_we_wont_spill();
    if (r != 0xFF) {
        regs[0] = r;
        regs[1] = r+1;
    } else {
        RTC_STACKCACHE_CLEAR_VALUETAG(REG_TO_ARRAY_INDEX(regs[0]));
    }
}
// Returns true if a register in the range >=r16 is allocated, false otherwise
bool rtc_stackcache_getfree_16bit_prefer_ge_R16(uint8_t *regs) {
    // Check if any reg in the range starting at R16 is free
    for(uint8_t reg=16; reg<=30; reg+=2) {
        uint8_t idx = REG_TO_ARRAY_INDEX(reg);
        if (RTC_STACKCACHE_IS_AVAILABLE(idx)) {
            // We're in luck.
            RTC_STACKCACHE_MARK_IN_USE(idx);
            RTC_STACKCACHE_CLEAR_VALUETAG(idx);
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

// PUSH
//    if we're pushing from R24, copy it to a free register first (calls GETFREE internally)
//    if the valuetag is set for the current instruction, record it
void rtc_stackcache_push_pair(uint8_t reg_base, uint8_t which_stack, bool is_int_l) {
    uint8_t idx = REG_TO_ARRAY_INDEX(reg_base);
    if (RTC_STACKCACHE_IS_IN_USE(idx)
    || (RTC_STACKCACHE_IS_AVAILABLE(idx) && (RTC_STACKCACHE_GET_VALUETAG(idx) == rtc_ts->current_instruction_valuetag
                                             || (RTC_STACKCACHE_GET_VALUETAG(idx) == RTC_VALUETAG_TO_INT_L(rtc_ts->current_instruction_valuetag) && is_int_l)))) {
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

        // set the value tag for the pushed value if we have one
        if (rtc_ts->current_instruction_valuetag != RTC_VALUETAG_UNUSED) {
            if (RTC_VALUETAG_IS_INT(rtc_ts->current_instruction_valuetag) && is_int_l) {
                // Special case when pushing the 2nd word of an int. Need to update the valuetag to be able to tell the high and low word apart.
                RTC_STACKCACHE_SET_VALUETAG(idx, RTC_VALUETAG_TO_INT_L(rtc_ts->current_instruction_valuetag));
            } else {
                RTC_STACKCACHE_SET_VALUETAG(idx, rtc_ts->current_instruction_valuetag);            
            }
        }
    } else {
        dj_panic(DJ_PANIC_AOT_STACKCACHE_PUSHED_REG_NOT_IN_USE);
    }
}
void rtc_stackcache_push_16bit(uint8_t *regs) {
    rtc_stackcache_push_pair(regs[0], RTC_STACKCACHE_INT_STACK_TYPE, false);
}
void rtc_stackcache_push_32bit(uint8_t *regs) {
    rtc_stackcache_push_pair(regs[2], RTC_STACKCACHE_INT_STACK_TYPE, false);
    rtc_stackcache_push_pair(regs[0], RTC_STACKCACHE_INT_STACK_TYPE, true);
}
void rtc_stackcache_push_ref(uint8_t *regs) {
    rtc_stackcache_push_pair(regs[0], RTC_STACKCACHE_REF_STACK_TYPE, false);
}
void rtc_stackcache_push_16bit_from_R24R25() {
    rtc_stackcache_push_pair(R24, RTC_STACKCACHE_INT_STACK_TYPE, false);
}
void rtc_stackcache_push_32bit_from_R22R25() {
    rtc_stackcache_push_pair(R24, RTC_STACKCACHE_INT_STACK_TYPE, false);
    rtc_stackcache_push_pair(R22, RTC_STACKCACHE_INT_STACK_TYPE, true);
}
void rtc_stackcache_push_ref_from_R24R25() {
    rtc_stackcache_push_pair(R24, RTC_STACKCACHE_REF_STACK_TYPE, false);
}

// POP
//    if nondestructive: pop, retaining the valuetag so it may be recycled later
//    if destructive: pop, and clear the valuetag since the calling instruction will destroy the value
//    if to_store: pop, and set the valuetag since we will write this value to memory. also clear any other register with the same value tag since it's no longer current
#define RTC_STACKCACHE_POP_NONDESTRUCTIVE 1
#define RTC_STACKCACHE_POP_DESTRUCTIVE    2
#define RTC_STACKCACHE_POP_TO_STORE       3
#define RTC_STACKCACHE_POP_TO_STORE_INT_L 4
void rtc_stackcache_pop_pair(uint8_t *regs, uint8_t poptype, uint8_t which_stack, uint8_t target_reg) {
    if (target_reg != 0xFF
     && target_reg != R18
     && target_reg != R20
     && target_reg != R22
     && target_reg != R24
     && target_reg != RZ) {
        while (true) {
         dj_panic(DJ_PANIC_AOT_STACKCACHE_INVALID_POP_TARGET); }
    }
    uint8_t target_idx = (target_reg == 0xFF) ? 0xFF : REG_TO_ARRAY_INDEX(target_reg);

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
                target_idx = rtc_get_lru_available_index();
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
                // The element at this stack depth is of the wrong type: try the next one.
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
                RTC_STACKCACHE_UPDATE_AGE(stack_top_idx); // Mark the fact that this value was used at this pc.
                RTC_STACKCACHE_SET_VALUETAG(target_idx, RTC_STACKCACHE_GET_VALUETAG(stack_top_idx)); // Also copy the valuetag if there is any
            }
        }

        RTC_STACKCACHE_UPDATE_AGE(target_idx); // Mark the fact that this value was used at this pc.
        if (target_idx != REG_TO_ARRAY_INDEX(RZ)) { // Don't mark Z in use, because it should't become available after this instruction
            RTC_STACKCACHE_MARK_IN_USE(target_idx); // Target is now IN USE (might already have been IN USE if we're popping to a specific register before a function call)
        }
        if (poptype == RTC_STACKCACHE_POP_NONDESTRUCTIVE) {
            // Just keep the valuetag since we may reuse it later
        } else if (poptype == RTC_STACKCACHE_POP_DESTRUCTIVE) {
            // The value will be destroyed, so clear the value tag
            RTC_STACKCACHE_CLEAR_VALUETAG(target_idx);
        } else if (poptype == RTC_STACKCACHE_POP_TO_STORE) {
            // The value will be stored to memory, so we should mark the value tag,
            // and also clear any other registers with the same tag since they no
            // longer contain the right value.
            rtc_poppedstackcache_clear_all_with_valuetag(rtc_ts->current_instruction_valuetag);
            RTC_STACKCACHE_SET_VALUETAG(target_idx, rtc_ts->current_instruction_valuetag);
        } else if (poptype == RTC_STACKCACHE_POP_TO_STORE_INT_L) {
            // The value will be stored to memory, so we should mark the value tag,
            // and also clear any other registers with the same tag since they no
            // longer contain the right value.
            rtc_poppedstackcache_clear_all_with_valuetag(RTC_VALUETAG_TO_INT_L(rtc_ts->current_instruction_valuetag));
            RTC_STACKCACHE_SET_VALUETAG(target_idx, RTC_VALUETAG_TO_INT_L(rtc_ts->current_instruction_valuetag));
        }

        if (regs != NULL) {
            regs[0] = ARRAY_INDEX_TO_REG(target_idx);
            regs[1] = ARRAY_INDEX_TO_REG(target_idx)+1;
        }
        return;
    }
}

void rtc_stackcache_pop_nondestructive_16bit(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_NONDESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_nondestructive_32bit(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_NONDESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
    rtc_stackcache_pop_pair(regs+2, RTC_STACKCACHE_POP_NONDESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_nondestructive_ref(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_NONDESTRUCTIVE, RTC_STACKCACHE_REF_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_destructive_16bit(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_destructive_32bit(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
    rtc_stackcache_pop_pair(regs+2, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_destructive_ref(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_REF_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_to_store_16bit(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_TO_STORE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_to_store_32bit(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_TO_STORE_INT_L, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
    rtc_stackcache_pop_pair(regs+2, RTC_STACKCACHE_POP_TO_STORE, RTC_STACKCACHE_INT_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_to_store_ref(uint8_t *regs) {
    rtc_stackcache_pop_pair(regs, RTC_STACKCACHE_POP_TO_STORE, RTC_STACKCACHE_REF_STACK_TYPE, 0xFF);
}
void rtc_stackcache_pop_destructive_16bit_into_fixed_reg(uint8_t reg_base) {         // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
    rtc_stackcache_pop_pair(NULL, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, reg_base);
}
void rtc_stackcache_pop_destructive_32bit_into_fixed_reg(uint8_t reg_base) {         // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
    rtc_stackcache_pop_pair(NULL, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, reg_base);
    rtc_stackcache_pop_pair(NULL, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_INT_STACK_TYPE, reg_base+2);
}
void rtc_stackcache_pop_destructive_ref_into_fixed_reg(uint8_t reg_base) {           // Pops a value into a specific range of consecutive regs. Panics if any reg is not marked IN USE.
    rtc_stackcache_pop_pair(NULL, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_REF_STACK_TYPE, reg_base);
}
void rtc_stackcache_pop_destructive_ref_into_Z() {                                   // Pops a value into Z. Panics if any reg is not marked IN USE.
    rtc_stackcache_pop_pair(NULL, RTC_STACKCACHE_POP_DESTRUCTIVE, RTC_STACKCACHE_REF_STACK_TYPE, RZ);
}

void rtc_stackcache_assert_no_in_use() {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_IN_USE(idx)) {
            dj_panic(DJ_PANIC_AOT_STACKCACHE_IN_USE);
        }
    }
}
void rtc_stackcache_flush_call_used_regs_and_clear_valuetags() {        // Pushes all call-used registers onto the stack, removing them from the cache (R18–R27, R30, R31), and clears the value tags since the value is about to be destroyed
    // Pushes all call-used registers onto the stack, removing them from the cache (R18–R25)
    rtc_stackcache_assert_no_in_use();

    // After this all call-used registers are marked IN USE, so they can be pushed on the stack to store a function result
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
            break; // No call-used regs are on the stack, so we're done.
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
            // There's also an available call-saved register, so move the value in the call-used reg there.
            emit_MOVW(ARRAY_INDEX_TO_REG(call_saved_reg_available_idx), ARRAY_INDEX_TO_REG(call_used_reg_on_stack_idx));
            // Update the cache state and value tag
            RTC_STACKCACHE_MOVE_CACHE_ELEMENT(call_saved_reg_available_idx, call_used_reg_on_stack_idx);
            RTC_STACKCACHE_SET_VALUETAG(call_saved_reg_available_idx, RTC_STACKCACHE_GET_VALUETAG(call_used_reg_on_stack_idx));
        }
    }

    // Finally, clear the valuetags for all call-used registers, since the value may be gone after the function call returns
    rtc_poppedstackcache_clear_all_callused_valuetags();
}

void rtc_stackcache_flush_all_regs() {
    // This is done before branches to make sure the whole stack is in memory. (Java guarantees the stack to be empty between statements, but there may still be things on the stack if this is part of a ? : expression.)
    // There's no need to clear the valuetags here, as we may reuse the values later if the branch wasn't taken. If it was, the valuetags are clears at the BRTARGET.
    while (get_deepest_pair_idx() != 0xFF) {
        rtc_stackcache_spill_deepest_pair();
    }
}

// Code to skip instructions if the valuetag is found
uint16_t rtc_stackcache_determine_valuetag(rtc_translationstate *ts, bool *instruction_produces_value_which_can_be_skipped) {
    uint8_t opcode = dj_di_getU8(ts->jvm_code_start + ts->pc);
    uint8_t jvm_operand_byte0 = dj_di_getU8(ts->jvm_code_start + ts->pc + 1);
    uint8_t next_opcode = jvm_operand_byte0;

    switch (opcode) {
        case JVM_ALOAD:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF + jvm_operand_byte0;
        case JVM_ASTORE:
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF + jvm_operand_byte0;
        case JVM_ALOAD_0:
        case JVM_ALOAD_1:
        case JVM_ALOAD_2:
        case JVM_ALOAD_3:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF + opcode - JVM_ALOAD_0;
        case JVM_ASTORE_0:
        case JVM_ASTORE_1:
        case JVM_ASTORE_2:
        case JVM_ASTORE_3:
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF + opcode - JVM_ASTORE_0;

        case JVM_SLOAD:
            *instruction_produces_value_which_can_be_skipped = true;
        case JVM_SSTORE:
        case JVM_SINC:
        case JVM_SINC_W:
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + jvm_operand_byte0;
        case JVM_SLOAD_0:
        case JVM_SLOAD_1:
        case JVM_SLOAD_2:
        case JVM_SLOAD_3:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + opcode - JVM_SLOAD_0;
        case JVM_SSTORE_0:
        case JVM_SSTORE_1:
        case JVM_SSTORE_2:
        case JVM_SSTORE_3:
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + opcode - JVM_SSTORE_0;

        case JVM_ILOAD:
            *instruction_produces_value_which_can_be_skipped = true;
        case JVM_ISTORE:
        case JVM_IINC:
        case JVM_IINC_W:
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT + jvm_operand_byte0;
        case JVM_ILOAD_0:
        case JVM_ILOAD_1:
        case JVM_ILOAD_2:
        case JVM_ILOAD_3:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT + opcode - JVM_ILOAD_0;
        case JVM_ISTORE_0:
        case JVM_ISTORE_1:
        case JVM_ISTORE_2:
        case JVM_ISTORE_3:
            return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT + opcode - JVM_ISTORE_0;

        case JVM_SCONST_1:
#ifdef AOT_OPTIMISE_CONSTANT_SHIFTS
            if (next_opcode == JVM_SSHL
                || next_opcode == JVM_SSHR
                || next_opcode == JVM_SUSHR
                || next_opcode == JVM_IUSHR) { // Somehow IUSHR has 16 bit operand but ISHR and ISHL have 32 bit.
                ts->do_CONST1_SHIFT_optimisation = true;
            }
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS
        case JVM_SCONST_M1:
        case JVM_SCONST_0:
        case JVM_SCONST_2:
        case JVM_SCONST_3:
        case JVM_SCONST_4:
        case JVM_SCONST_5:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_CONSTANT + RTC_VALUETAG_DATATYPE_SHORT + opcode - JVM_SCONST_M1;

        case JVM_ICONST_1:
#ifdef AOT_OPTIMISE_CONSTANT_SHIFTS
            if (next_opcode == JVM_ISHL
                || next_opcode == JVM_ISHR) { // Somehow IUSHR has 16 bit operand but ISHR and ISHL have 32 bit.
                ts->do_CONST1_SHIFT_optimisation = true;
            }
#endif // AOT_OPTIMISE_CONSTANT_SHIFTS
        case JVM_ICONST_M1:
        case JVM_ICONST_0:
        case JVM_ICONST_2:
        case JVM_ICONST_3:
        case JVM_ICONST_4:
        case JVM_ICONST_5:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_CONSTANT + RTC_VALUETAG_DATATYPE_INT + opcode - JVM_ICONST_M1;

        case JVM_ACONST_NULL:
            *instruction_produces_value_which_can_be_skipped = true;
            return RTC_VALUETAG_TYPE_CONSTANT + RTC_VALUETAG_DATATYPE_REF + 0;

        default:
            return RTC_VALUETAG_UNUSED;
    }
}

uint8_t rtc_poppedstackcache_find_available_valuetag(uint16_t valuetag) {
    for (uint8_t idx=0; idx<RTC_STACKCACHE_MAX_IDX; idx++) {
        if (RTC_STACKCACHE_IS_AVAILABLE(idx) && RTC_STACKCACHE_GET_VALUETAG(idx) == valuetag) {
            return idx;
        }
    }
    return 0xFF;
}

bool rtc_poppedstackcache_can_I_skip_this() {
    uint8_t idx, idx_l = 0xFF;
    bool instruction_produces_value_which_can_be_skipped = false;

    rtc_ts->current_instruction_pc = rtc_ts->pc; // Save this because we may need in later after the instruction already increased pc to skip over arguments
    rtc_ts->current_instruction_valuetag = rtc_stackcache_determine_valuetag(rtc_ts, &instruction_produces_value_which_can_be_skipped);


    if (instruction_produces_value_which_can_be_skipped) {
        if (rtc_ts->do_CONST1_SHIFT_optimisation) {
            return true; // Skip the CONST1 and let the next shift instruction shift by 1 bit.
        }

        // Check if there is an available register that contains the value we need (because it was loaded and the popped earlier in this basic block)
        idx = rtc_poppedstackcache_find_available_valuetag(rtc_ts->current_instruction_valuetag);
        if (idx == 0xFF) {
            return false;
        }

        if (RTC_VALUETAG_IS_INT(rtc_ts->current_instruction_valuetag)) {
            // If it's an int, we also need to find the other half
            idx_l = rtc_poppedstackcache_find_available_valuetag(RTC_VALUETAG_TO_INT_L(rtc_ts->current_instruction_valuetag));
            if (idx_l == 0xFF) {
                return false;
            }
        }

        // Found it!
        uint8_t operand_regs[4];
        if (RTC_VALUETAG_IS_INT(rtc_ts->current_instruction_valuetag)) {
            operand_regs[0] = ARRAY_INDEX_TO_REG(idx_l);
            operand_regs[1] = ARRAY_INDEX_TO_REG(idx_l)+1;
            operand_regs[2] = ARRAY_INDEX_TO_REG(idx);
            operand_regs[3] = ARRAY_INDEX_TO_REG(idx)+1;
            rtc_stackcache_push_32bit(operand_regs);
        } else {
            operand_regs[0] = ARRAY_INDEX_TO_REG(idx);
            operand_regs[1] = ARRAY_INDEX_TO_REG(idx)+1;
            if (RTC_VALUETAG_IS_REF(rtc_ts->current_instruction_valuetag)) {
                rtc_stackcache_push_ref(operand_regs);
            } else if (RTC_VALUETAG_IS_SHORT(rtc_ts->current_instruction_valuetag)) {
                rtc_stackcache_push_16bit(operand_regs);
            }
        }

        return true;
    } else {
        return false;
    }
}


#endif // AOT_STRATEGY_POPPEDSTACKCACHE
