#include "asm.h"
#include "rtc_emit.h"
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



// NOTE THAT THIS CODE ONLY WORKS ON AVR DEVICES WITH <=128KB flash.
// On larger devices, such as the WuDevice, the return address after
// a call is 24 bit, so we need to pop/push 3 bytes at the beginning
// and end of the fragments below.
// (POP R18; POP19 -> POP R18; POP R19; POP R20)

// Call saved: r1, r2-r17, r28:r29 (Y)
const uint16_t PROGMEM emit_x_prologue_code[] =
{ asm_const_POP(R18),
  asm_const_POP(R19),
  asm_const_PUSH(R2),
  asm_const_PUSH(R3),
  asm_const_PUSH(R4),
  asm_const_PUSH(R5),
  asm_const_PUSH(R6),
  asm_const_PUSH(R7),
  asm_const_PUSH(R8),
  asm_const_PUSH(R9),
  asm_const_PUSH(R10),
  asm_const_PUSH(R11),
  asm_const_PUSH(R12),
  asm_const_PUSH(R13),
  asm_const_PUSH(R14),
  asm_const_PUSH(R15),
  asm_const_PUSH(R16),
  asm_const_PUSH(R17),
  asm_const_PUSH(R28),
  asm_const_PUSH(R29),
  asm_const_MOVW(R28, R24),
  asm_const_MOVW(R26, R22),
  asm_const_MOVW(R2, R20),
  asm_const_PUSH(R19),
  asm_const_PUSH(R18),
  OPCODE_RET };
void emit_x_prologue() {
    emit_2_CALL(((uint16_t)emit_x_prologue_code)/2);
}

// Call saved: r1, r2-r17, r28:r29 (Y)
const uint16_t PROGMEM emit_x_epilogue_code[] =
{ asm_const_POP(R18),
  asm_const_POP(R19),
  asm_const_POP(R29),
  asm_const_POP(R28),
  asm_const_POP(R17),
  asm_const_POP(R16),
  asm_const_POP(R15),
  asm_const_POP(R14),
  asm_const_POP(R13),
  asm_const_POP(R12),
  asm_const_POP(R11),
  asm_const_POP(R10),
  asm_const_POP(R9),
  asm_const_POP(R8),
  asm_const_POP(R7),
  asm_const_POP(R6),
  asm_const_POP(R5),
  asm_const_POP(R4),
  asm_const_POP(R3),
  asm_const_POP(R2),
  asm_const_PUSH(R19),
  asm_const_PUSH(R18),
  OPCODE_RET };
void emit_x_epilogue() {
    emit_2_CALL(((uint16_t)emit_x_epilogue_code)/2);
    emit_RET();
}
