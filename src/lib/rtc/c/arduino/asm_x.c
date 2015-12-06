#include "asm.h"
#include "rtc_emit.h"


void emit_x_CALL(uint16_t target) {
    // Flush the code buffer before emitting a CALL to prevent PUSH/POP pairs being optimised across a CALL instruction.
    emit_PUSH(RXH);
    emit_PUSH(RXL);
    emit_flush_to_flash();
    emit_2_CALL(target);
    emit_POP(RXL);
    emit_POP(RXH);
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

void emit_x_prologue() {
    // prologue (is this the right way?)
    emit_PUSH(R3);
    emit_PUSH(R2);
    emit_PUSH(R29); // Push Y
    emit_PUSH(R28);
    emit_MOVW(R28, R24); // Pointer to locals in Y
    emit_MOVW(R26, R22); // Pointer to ref stack in X
    emit_MOVW(R2, R20); // Pointer to static in R2 (will be MOVWed to R30 when necessary)
}

void emit_x_epilogue() {
    // epilogue (is this the right way?)
    emit_POP(R28); // Pop Y
    emit_POP(R29);
    emit_POP(R2);
    emit_POP(R3);
    emit_RET();
}

