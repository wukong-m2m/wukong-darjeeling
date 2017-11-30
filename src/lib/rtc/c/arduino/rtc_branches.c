#include <stdint.h>
#include "panic.h"
#include "program_mem.h"
#include "wkreprog.h"
#include "asm.h"
#include "parse_infusion.h"
#include "rtc.h"
#include "rtc_emit.h"
#include "rtc_branches.h"
#include "rtc_safetychecks.h"
#include "rtc_safetychecks_opcodes.h"
#include "config.h"


void rtc_mark_branchtarget() {
    // This is a noop, but we need to record the offset of the next
    // instruction in the branch target table at the start of the method.
    // Record the offset IN WORDS from the method start to make sure it works
    // for addresses > 128K as well. (but this won't work for methods > 128K)
    emit_flush_to_flash(); // Finish writing, and also make sure we won't optimise across basic block boundaries.
    uint_farptr_t tmp_current_position = wkreprog_get_raw_position();
    wkreprog_close();
    wkreprog_open_raw(rtc_branch_target_table_1_address(rtc_ts, rtc_ts->branch_target_count), RTC_END_OF_COMPILED_CODE_SPACE);
    emit_raw_word(tmp_current_position/2 - rtc_ts->branch_target_table_start_ptr/2);
    emit_flush_to_flash();
    wkreprog_close();
    wkreprog_open_raw(tmp_current_position, RTC_END_OF_COMPILED_CODE_SPACE);
    rtc_ts->branch_target_count++;
}

void emit_x_branchtag(uint16_t opcode, uint16_t target) {
    // instead of the branch, output a tag that can be replaced when target addresses are all known:
    //   16 bit branch tag
    //   16 bit desired branch opcode, without offset (may be changed to different branch if out of range)
    //   16 bit branch target id
    emit_flush_to_flash(); // To make sure we won't accidentally optimise addresses or branch labels
    emit_BREAK();
    emit(opcode);
    emit_raw_word(target);
    emit_flush_to_flash(); // To make sure we won't accidentally optimise addresses or branch labels

#ifdef AOT_SAFETY_CHECKS
    if (!RTC_OPCODE_IS_RETURN(rtc_ts->current_opcode) // For returns we use an extra branchtarget added to the end of each method. Since returns only jump to this target, they are always safe.
            && target >= dj_di_methodImplementation_getNumberOfBranchTargets(rtc_ts->methodimpl)) {
        rtc_safety_abort_with_error(RTC_SAFETYCHECK_BRANCH_TO_NONEXISTANT_BRTARGET);
    }
#endif //AOT_SAFETY_CHECKS
}

#define RTC_STARTADDRESS_BRTARGET_TABLE_1 (branch_target_table_start_ptr)
#define RTC_STARTADDRESS_BRTARGET_TABLE_2 (branch_target_table_start_ptr + branchTableSize/2)

// Note we only compensate for shift_because_of_smaller_prologue when reading from table 1.
// This is because those addresses were filled during code gen, when the prologue size was not yet known.
// After code gen, all generated code was shifted back if we emitted a smaller prologue, so we compensate by
// substracting shift_because_of_smaller_prologue.
// The addresses in table 2 are taken from addresses we got from RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_1
// so these are already adjusted for the smaller prologue.
#define RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_1(branchtarget_id) (branch_target_table_start_ptr - shift_because_of_smaller_prologue + 2*dj_di_getU16(RTC_STARTADDRESS_BRTARGET_TABLE_1 + 2*branchtarget_id))
#define RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_2(branchtarget_id) (branch_target_table_start_ptr + 2*dj_di_getU16(RTC_STARTADDRESS_BRTARGET_TABLE_2 + 2*branchtarget_id))
void rtc_patch_branches(uint_farptr_t branch_target_table_start_ptr, uint_farptr_t end_of_method, uint16_t branchTableSize, uint8_t shift_because_of_smaller_prologue) {
    // All branchtarget addresses should be known now, but they are based on each branch taking 3 words.
    // (since the worst case is a 1 word (reverse) conditional branch, follow by a 2 word JMP)
    // The NOP that we would need to generate take up over 10% in some benchmarks (bsort, binsrch), so
    // we will first do a (few) passes to see how much each branch will really take and update the branch
    // targets accordingly. One pass may not be enough, since replacing one branch with a smaller version
    // could enable another to use a smaller version as well.

    // BT 1: original addresses, based on 3 word branch
    // BT 2: optimised addresses, based on as small a branch as possible
    // bool terminate = true
    // do
    //     int savings = 0
    //     for each instruction until we've passed all branchtargets
    //         if current address == next BT 1 address then
    //             if next BT 2 address != current address - savings then
    //                 set terminate = false
    //                 update BT 2 address to current address - savings
    //                 next BT ++
    //             fi
    //         fi
    //         if current address is a branch then
    //             savings + 3 - the number of bytes necessary to branch from current address to the address currently stored in BT 2
    //         fi
    //     rof
    // while not terminate
    // after this BT 2 will contain the correct addresses if we emit the shortest possible branch

    // initialise BT 2 with a copy of BT 1.
    uint16_t branch_target_count = branchTableSize/4;

    bool terminate = true;
    do {
        terminate = true;
        uint16_t next_branch_target_idx = 0;
        uint16_t savings = 0;
        wkreprog_open_raw(RTC_STARTADDRESS_BRTARGET_TABLE_2, end_of_method);
    //     for each instruction
        uint_farptr_t avr_pc = branch_target_table_start_ptr + branchTableSize;
    //                          until we've passed all branchtargets
        while (next_branch_target_idx < branch_target_count) {
    //         if current address == next BT 1 address then
            while (avr_pc == RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_1(next_branch_target_idx)) { // we need a while loop here in case there are multiple BRTARGETs with the same address. (ex: BRTARGET, MARKLOOP_END, BRTARGET, when we don't use the MARKLOOP information)
    //             if next BT 2 address != current address - savings then
                if ((avr_pc - savings) != RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_2(next_branch_target_idx)) {
    //                 set terminate = false
                    terminate = false; // If we're going to update the value address in table 2 (in a second pass), we should do another pass to check for further optimisations
                }
    //                 update BT 2 address to current address - savings
                emit_without_optimisation((avr_pc - savings - branch_target_table_start_ptr)/2); // Store offset in words
    //                 next BT ++
                next_branch_target_idx++;
                if (next_branch_target_idx == branch_target_count) // Start of table 2 may point to this avr_pc, so stop when we're scrolling out of table 1.
                    break;
            }

    //         if current address is a branch then
            uint16_t avr_instruction = dj_di_getU16(avr_pc);
            if (rtc_is_branchtag(avr_instruction)) {
    //             savings + 3 - the number of bytes necessary to branch from current address to the address currently stored in BT 2
                uint16_t avr_instruction = dj_di_getU16(avr_pc+2);
                uint16_t branchtarget_id = dj_di_getU16(avr_pc+4);
                uint_farptr_t target_address_in_bytes = RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_2(branchtarget_id);
                int16_t target_offset_in_bytes = target_address_in_bytes - avr_pc + savings - 2; // +savings to compensate for the fact this instruction may be at a different location, so the offset will be different
                avr_pc+=4;

                if (avr_instruction == OPCODE_RJMP) {
                    if (target_offset_in_bytes >= -4096 && target_offset_in_bytes <= 4094) {
                        savings += 2*2; // RJMP -> saves 2/3 words
                    } else {
                        savings += 1*2; // JMP -> saves 1/3 words
                    }                
                } else {
                    if (target_offset_in_bytes >= -128 && target_offset_in_bytes <= 126) {
                        savings += 2*2; // BR__ -> saves 2/3 words
                    } else if (target_offset_in_bytes >= -4096 && target_offset_in_bytes <= 4094) {
                        savings += 1*2; // reversed BR__, RJMP -> saves 1/3 words
                    } else {
                        savings += 0*2; // reversed BR__, JMP -> saves 0/3 words (worst case)
                    }
                }
            } else {
                if (rtc_is_double_word_instruction(avr_instruction)) {
                    avr_pc += 2;
                }
            }
            avr_pc += 2;
        }
        wkreprog_close();
    } while (!terminate);

    // avroraPrintStr("-------BEFORE NOP OPT:");
    // for (uint16_t i = 0; i<branch_target_count; i++) {
    //     avroraPrintHex32(RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_1(i));
    // }
    // avroraPrintStr("-------AFTER NOP OPT:");
    // for (uint16_t i = 0; i<branch_target_count; i++) {
    //     avroraPrintHex32(RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_2(i));
    // }


    // Scan for branch tags, and replace them with the proper instructions.
    wkreprog_open_raw(branch_target_table_start_ptr, end_of_method);
    wkreprog_skip(branchTableSize);

    for (uint_farptr_t avr_pc=branch_target_table_start_ptr+branchTableSize; avr_pc<end_of_method; avr_pc += 2) {
        uint16_t avr_instruction = dj_di_getU16(avr_pc);

        if (rtc_is_branchtag(avr_instruction)) {
            // BREQ  1111 00kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
            // RJMP  1100 kkkk kkkk kkkk, with k the signed offset to jump to, in WORDS, not bytes. PC <- PC + k + 1
            // JMP   1001 010k kkkk 110k kkkk kkkk kkkk kkkk, with k the address in WORDS, not bytes. PC <- k
            // Max reach for BREQ, etc: -128 +126 BYTES
            // Max reach for RJMP: -4096 +4094 BYTES
            // AVR opcode will be BREQ, BRNE, BRLT, BRGE, or RJMP

            // Replace with real branch instruction
            // There are two more words in the branch tag:
            //   the desired branch opcode, 
            //   and the branchtarget id
            uint16_t avr_instruction = dj_di_getU16(avr_pc+2);
            uint16_t branchtarget_id = dj_di_getU16(avr_pc+4);
            uint_farptr_t target_address_in_bytes = RTC_GET_BRTARGET_BYTE_ADDRESS_FROM_TABLE_2(branchtarget_id); // Get the target of the branch from the branch table
            int16_t target_offset_in_bytes = target_address_in_bytes - wkreprog_impl_get_raw_position() - 2; // -2 because the result is PC<-PC+k+1 (in words). Without the -2 we would end up 2 bytes/1 word too far
            avr_pc+=4;


            if (avr_instruction == OPCODE_RJMP) {
                // Unconditional branch
                if (target_offset_in_bytes >= -4096 && target_offset_in_bytes <= 4094) {
                    emit_RJMP(target_offset_in_bytes);
                } else {
                    emit_2_JMP(target_address_in_bytes);
                }                
            } else {
                // Conditional branch
                if (target_offset_in_bytes >= -128
                        && target_offset_in_bytes <= 126) {
                    // Fits in a normal BR__ instruction.
                    switch (avr_instruction) {
                        case OPCODE_BREQ: emit_BREQ(target_offset_in_bytes); break;
                        case OPCODE_BRNE: emit_BRNE(target_offset_in_bytes); break;
                        case OPCODE_BRLO: emit_BRLO(target_offset_in_bytes); break;
                        case OPCODE_BRLT: emit_BRLT(target_offset_in_bytes); break;
                        case OPCODE_BRGE: emit_BRGE(target_offset_in_bytes); break;
                        default:
                            dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
                    }
                } else {
                    // Doesn't fit in BR__ instruction, reverse the condition and branch over an unconditional RJMP or JMP.
                    bool useRJMP = target_offset_in_bytes >= -4096 && target_offset_in_bytes <= 4094;
                    uint8_t offset = useRJMP ? 2 : 4;
                    switch (avr_instruction) {
                        case OPCODE_BREQ: emit_BRNE(offset); break;
                        case OPCODE_BRNE: emit_BREQ(offset); break;
                        case OPCODE_BRLO: emit_BRSH(offset); break;
                        case OPCODE_BRLT: emit_BRGE(offset); break;
                        case OPCODE_BRGE: emit_BRLT(offset); break;
                        default:
                            dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
                    }
                    if (useRJMP) {
                        emit_RJMP(target_offset_in_bytes-2);
                    } else {
                        emit_2_JMP(target_address_in_bytes);
                    }
                }
            }
        } else {
            // Normal instruction: just copy it
            // Note that we can't optimise here for two reasons:
            //   optimising would misalign the branch target addresses
            //   the optimiser doesn't consider branches, so we could end up optimising across basic block boundaries, with would result in illegal code
            emit_without_optimisation(avr_instruction);
            if (rtc_is_double_word_instruction(avr_instruction)) {
                // Copy anoher word for double word instructions
                avr_pc += 2;
                emit_without_optimisation(dj_di_getU16(avr_pc));
            }
        }
        emit_flush_to_flash();
    }
}