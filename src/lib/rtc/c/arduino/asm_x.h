#ifndef ASM_X_H
#define ASM_X_H

#include <stdint.h>

// PUSHREF
#define emit_x_PUSHREF8(reg)              emit_ST_XINC(reg)
#define emit_x_POPREF8(reg)               emit_LD_DECX(reg)

void emit_x_CALL(uint16_t target);
void emit_x_POP_32bit(uint8_t base);
void emit_x_POP_16bit(uint8_t base);
void emit_x_POP_REF(uint8_t base);
void emit_x_PUSH_32bit(uint8_t base);
void emit_x_PUSH_16bit(uint8_t base);
void emit_x_PUSH_REF(uint8_t base);
void emit_x_prologue();
void emit_x_epilogue();
void emit_x_preinvoke();
void emit_x_postinvoke();

#endif // ASM_X_H