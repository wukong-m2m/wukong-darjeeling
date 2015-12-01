#include <stdint.h>
#include <stdbool.h>

bool rtc_is_double_word_instruction(uint16_t instruction);
void emit_STD(uint8_t reg, uint8_t xy, uint8_t offset);
void emit_LDD(uint8_t reg, uint8_t xy, uint8_t offset);
void emit_LDI(uint8_t reg, uint8_t constant);
void emit_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg);
void emit_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg);
void emit_BRANCH(uint16_t opcode, uint8_t offset);
void emit_ADIW(uint8_t reg, uint8_t constant);
uint16_t asm_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg);
uint16_t asm_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg);
