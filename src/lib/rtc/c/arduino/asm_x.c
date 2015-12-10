#include "asm.h"
#include "rtc_emit.h"

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
// Call saved: r1, r2-r17, r28:r29 (Y)
void emit_x_prologue() {
    // prologue (is this the right way?)
    emit_PUSH(R2);
    emit_PUSH(R3);
    emit_PUSH(R4);
    emit_PUSH(R5);
    emit_PUSH(R6);
    emit_PUSH(R7);
    emit_PUSH(R8);
    emit_PUSH(R9);
    emit_PUSH(R10);
    emit_PUSH(R11);
    emit_PUSH(R12);
    emit_PUSH(R13);
    emit_PUSH(R14);
    emit_PUSH(R15);
    emit_PUSH(R16);
    emit_PUSH(R17);
    emit_PUSH(R28);
    emit_PUSH(R29); // Push Y
    emit_MOVW(R28, R24); // Pointer to locals in Y
    emit_MOVW(R26, R22); // Pointer to ref stack in X
    emit_MOVW(R2, R20); // Pointer to static in R2 (will be MOVWed to R30 when necessary)
}

void emit_x_epilogue() {
    // epilogue (is this the right way?)
    emit_POP(R29); // Pop Y
    emit_POP(R28);
    emit_POP(R17);
    emit_POP(R16);
    emit_POP(R15);
    emit_POP(R14);
    emit_POP(R13);
    emit_POP(R12);
    emit_POP(R11);
    emit_POP(R10);
    emit_POP(R9);
    emit_POP(R8);
    emit_POP(R7);
    emit_POP(R6);
    emit_POP(R5);
    emit_POP(R4);
    emit_POP(R3);
    emit_POP(R2);

    emit_RET();
}
