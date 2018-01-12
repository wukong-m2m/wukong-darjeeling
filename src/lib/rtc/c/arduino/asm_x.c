#include "asm.h"
#include "rtc_emit.h"
#include "config.h"
#include "execution.h"
#include <avr/pgmspace.h>

// push pop order
// Ints: Push 1, Push 0     Pop 0, Pop 1 (Ints are stored LE in the registers, so this push order stores them BE in memory since int stack grows down. This is what DJ expects)
// Refs: Push 0, Push 1     Pop 1, Pop 0

void emit_x_CALL(uint16_t target) {
    // Flush the code buffer before emitting a CALL to prevent PUSH/POP pairs being optimised across a CALL instruction.
    emit_PUSH(RXH);
    emit_PUSH(RXL);
    emit_flush_to_flash();
    emit_2_CALL(target);
    emit_POP(RXL);
    emit_POP(RXH);
}

void emit_x_PUSH_32bit(uint8_t base) {
    emit_PUSH(base+3);
    emit_PUSH(base+2);
    emit_PUSH(base+1);
    emit_PUSH(base+0);
}
void emit_x_PUSH_16bit(uint8_t base) {
    emit_PUSH(base+1);
    emit_PUSH(base+0);
}
void emit_x_PUSH_REF(uint8_t base) {
    emit_x_PUSHREF8(base+0);
    emit_x_PUSHREF8(base+1);
}

void emit_x_POP_32bit(uint8_t base) {
    emit_POP(base+0);
    emit_POP(base+1);
    emit_POP(base+2);
    emit_POP(base+3);
}
void emit_x_POP_16bit(uint8_t base) {
    emit_POP(base+0);
    emit_POP(base+1);
}
void emit_x_POP_REF(uint8_t base) {
    emit_x_POPREF8(base+1);
    emit_x_POPREF8(base+0);
}
void emit_x_avroraBeep(uint8_t beep) {
    emit_PUSH(R24);
    emit_LDI(R24, beep);
    emit_2_STS((uint16_t)&rtcMonitorVariable+1, R24);
    emit_LDI(R24, AVRORA_RTC_BEEP);
    emit_2_STS((uint16_t)&rtcMonitorVariable, R24);
    emit_POP(R24);
}
void emit_x_avroraPrintPC() {
    emit_PUSH(R24);
    emit_LDI(R24, 0x12);
    emit_2_STS((uint16_t)&(debugbuf1), R24);
    emit_POP(R24);
}
void emit_x_avroraPrintRegs() {
    emit_PUSH(R30);
    emit_LDI(R30, 0xE);
    emit_2_STS((uint16_t)&(debugbuf1), R30);
    emit_POP(R30);
}

// This needs to be a #define to calculate the instruction at compile time.
// There is also a asm_opcodeWithSingleRegOperand, which is a function. This
// needs to be a function to save programme size, because expanding this macro
// with variable parameters will be much larger than the function calls.
#define asm_const_opcodeWithSingleRegOperand(opcode, reg) (((opcode) + ((reg) << 4)))

// NOTE THAT THIS CODE ONLY WORKS ON AVR DEVICES WITH <=128KB flash.
// On larger devices, such as the WuDevice, the return address after
// a call is 24 bit, so we need to pop/push 3 bytes at the beginning
// and end of the fragments below.
// (POP R18; POP19 -> POP R18; POP R19; POP R20)
void emit_x_preinvoke_code() {

    asm volatile("       pop  r18" "\n\r"                   
                 "       pop  r19" "\n\r"
                                                                          // set intStack to SP 
                 "       push r1" "\n\r"                                  // NOTE: THE DVM STACK IS A 16 BIT POINTER, SP IS 8 BIT.
                                                                          // BOTH POINT TO THE NEXT free SLOT, BUT SINCE THEY GROW down THIS MEANS THE DVM POINTER SHOULD POINT TO TWO BYTES BELOW THE LAST VALUE,
                                                                          // WHILE CURRENTLY THE NATIVE SP POINTS TO THE BYTE DIRECTLY BELOW IT. RESERVE AN EXTRA BYTE TO FIX THIS.
                 "       in   r0, %[_SP_L_port]" "\n\r"                   // Load SPL into R0
                 "       sts  intStack, r0" "\n\r"                        // Store R0 into intStackL
                 "       in   r0, %[_SP_H_port]" "\n\r"                   // Load SPH into R0
                 "       sts  intStack+1, r0" "\n\r"                      // Store R0 into intStackH
                                                                          // Reserve 8 bytes of space on the stack, in case the returned int is large than passed ints
                                                                          // TODO: make this more efficient by looking up the method, and seeing if the return type is int,
                                                                          //       and if so, if the size of the return type is larger than the integers passed. Then only
                                                                          //       reserve the space that's needed.
                                                                          //       This is for the worst case, where no ints are passed, so there's no space reserved, and
                                                                          //       a 32 bit long is returned.
                 "       rcall next1" "\n\r"                              // RCALL to offset 0 does nothing, except reserving 2 bytes on the stack. cheaper than two useless pushes.
                 "next1: rcall next2" "\n\r"
                                                                          // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
                 "next2: sts  refStack, r26" "\n\r"
                 "       sts  refStack+1, r27" "\n\r"
                 "       push r19" "\n\r"
                 "       push r18" "\n\r"
             :: [_SP_L_port] "M" (SP_L_port), [_SP_H_port] "M" (SP_H_port));
}

void emit_x_preinvoke() {
    emit_2_CALL((uint16_t)&emit_x_preinvoke_code);
}

void emit_x_postinvoke_code() {

    asm volatile("       pop  r18" "\n\r"                   
                 "       pop  r19" "\n\r"

#ifndef EXECUTION_FRAME_ON_STACK
                                                                          // Y is call-saved and won't move if the frame is on the stack
                                                                          // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                 "       lds  r28, localReferenceVariables" "\n\r"        // Load localReferenceVariables into Y
                 "       lds  r29, localReferenceVariables+1" "\n\r"      // Load localReferenceVariables into Y
#endif
                                                                          // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                 "       lds  r26, refStack" "\n\r"                       // Load refStack into X
                 "       lds  r27, refStack+1" "\n\r"                     // Load refStack into X
                                                                          // get SP from intStack
                 "       lds  r0, intStack" "\n\r"                        // Load intStackL into R0
                 "       out  %[_SP_L_port], r0" "\n\r"                   // Store R0 into SPL
                 "       lds  r0, intStack+1" "\n\r"                      // Load intStackL into R0
                 "       out  %[_SP_H_port], r0" "\n\r"                   // Store R0 into SPH
                 "       pop  r0" "\n\r"                                  // JUST POP AND DISCARD TO CLEAR THE BYTE WE RESERVED IN THE asm_const_PUSH(ZERO_REG) LINE IN PREINVOKE
                 "       push r19" "\n\r"
                 "       push r18" "\n\r"
             :: [_SP_L_port] "M" (SP_L_port), [_SP_H_port] "M" (SP_H_port));
}

void emit_x_postinvoke() {
    emit_2_CALL((uint16_t)&emit_x_postinvoke_code);
}
