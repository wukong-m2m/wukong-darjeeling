#include <stdint.h>
#include "panic.h"
#include "program_mem.h"
#include "wkreprog.h"
#include "asm.h"
#include "rtc_emit.h"
#include "rtc_branches.h"

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
}

void rtc_patch_branches(dj_di_pointer branch_target_table_start_ptr, dj_di_pointer end_of_safe_region, uint16_t branchTableSize, dj_di_pointer end_of_method) {
// Let RTC trace know the next emitted code won't belong to the last JVM opcode anymore. Otherwise the branch patches would be assigned to the last instruction in the method (probably RET)
#ifdef AVRORA
    avroraRTCTracePatchingBranchesOn();
#endif

    // All branchtarget addresses should be known now.
    // Scan for branch tags, and replace them with the proper instructions.
    wkreprog_open_raw(branch_target_table_start_ptr, end_of_safe_region);
    wkreprog_skip(branchTableSize);

    for (uint16_t avr_pc=branch_target_table_start_ptr+branchTableSize; avr_pc<end_of_method; avr_pc+=2) {
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
            dj_di_pointer target_address_in_bytes = branch_target_table_start_ptr + 2*dj_di_getU16(branch_target_table_start_ptr + 2*branchtarget_id); // Get the target of the branch from the branch table
            int16_t target_offset_in_bytes = target_address_in_bytes - avr_pc - 2; // -2 because the result is PC<-PC+k+1 (in words). Without the -2 we would end up 2 bytes/1 word too far
            avr_pc+=4;

            if (avr_instruction == OPCODE_RJMP) {
                // Unconditional branch
                if (target_offset_in_bytes >= -4096 && target_offset_in_bytes <= 4094) {
                    emit_RJMP(target_offset_in_bytes);
                    emit_NOP(); // Fill to 3 words too keep brtarget addresses correct
                    emit_NOP();
                } else {
                    emit_2_JMP(target_address_in_bytes);
                    emit_NOP(); // Fill to 3 words too keep brtarget addresses correct
                }                
            } else {
                // Conditional branch
                if (target_offset_in_bytes >= -128
                        && target_offset_in_bytes <= 126) {
                    // Fits in a normal BR__ instruction.
                    switch (avr_instruction) {
                        case OPCODE_BREQ: emit_BREQ(target_offset_in_bytes); break;
                        case OPCODE_BRNE: emit_BRNE(target_offset_in_bytes); break;
                        case OPCODE_BRLT: emit_BRLT(target_offset_in_bytes); break;
                        case OPCODE_BRGE: emit_BRGE(target_offset_in_bytes); break;
                        default:
                            dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
                    }
                    emit_NOP();
                    emit_NOP();
                } else {
                    // Doesn't fit in BR__ instruction, reverse the condition and branch over an unconditional RJMP or JMP.
                    switch (avr_instruction) {
                        case OPCODE_BREQ: emit_BRNE(4); break;
                        case OPCODE_BRNE: emit_BREQ(4); break;
                        case OPCODE_BRLT: emit_BRGE(4); break;
                        case OPCODE_BRGE: emit_BRLT(4); break;
                        default:
                            dj_panic(DJ_PANIC_UNSUPPORTED_OPCODE);
                    }
                    if (target_offset_in_bytes >= -4096 && target_offset_in_bytes <= 4094) {
                        emit_RJMP(target_offset_in_bytes-2);
                        emit_NOP();
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
                avr_pc+=2;
                emit_without_optimisation(dj_di_getU16(avr_pc));
            }
        }
    }

// Let RTC trace know we're done patching branches
#ifdef AVRORA
    avroraRTCTracePatchingBranchesOff();
#endif
}