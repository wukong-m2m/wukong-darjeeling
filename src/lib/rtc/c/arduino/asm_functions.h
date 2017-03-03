#ifndef RTC_ASM_FUNCTIONS_H
#define RTC_ASM_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>

bool rtc_is_double_word_instruction(uint16_t instruction);

uint16_t asm_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg);
void emit_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg);
uint16_t asm_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg);
void emit_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg);

void emit_ADIW(uint8_t reg, uint8_t constant);
void emit_BRANCH(uint16_t opcode, uint8_t offset);
uint16_t emit_ADIW_if_necessary_to_bring_offset_in_range(uint8_t reg, uint16_t offset);
void emit_LDD(uint8_t reg, uint8_t yz, uint16_t offset);
void emit_STD(uint8_t reg, uint8_t yz, uint16_t offset);
void emit_LDI(uint8_t reg, uint8_t constant);
void emit_SBCI(uint8_t reg, uint8_t constant);
void emit_SUBI(uint8_t reg, uint8_t constant);
uint16_t asm_MOVW(uint8_t destreg, uint8_t srcreg);
void emit_MOVW(uint8_t destreg, uint8_t srcreg);


#endif // RTC_ASM_FUNCTIONS_H