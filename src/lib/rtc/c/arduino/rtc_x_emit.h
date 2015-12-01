// PUSHREF
#define emit_x_PUSHREF8(reg)              emit_ST_XINC(reg)
#define emit_x_POPREF8(reg)               emit_LD_DECX(reg)

void emit_x_CALL(uint16_t target) {
    // Flush the code buffer before emitting a CALL to prevent PUSH/POP pairs being optimised across a CALL instruction.
    emit_PUSH(RXH);
    emit_PUSH(RXL);
    rtc_flush();
    emit2( asm_CALL1(target) , asm_CALL2(target) );
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

void emit_x_epilogue() {
    // epilogue (is this the right way?)
    emit_POP(R28); // Pop Y
    emit_POP(R29);
    emit_POP(R2);
    emit_POP(R3);
    emit_RET();
}

