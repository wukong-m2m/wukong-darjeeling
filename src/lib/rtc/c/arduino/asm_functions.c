#include <stdint.h>
#include <stdbool.h>
#include "asm.h"
#include "emit.h"

bool rtc_is_double_word_instruction(uint16_t instruction) {
    const uint16_t CALL_MASK            = 0xFE0E;
    const uint16_t LDS_MASK             = 0xFE0F;
    const uint16_t STS_MASK             = 0xFE0F;

    return (instruction & CALL_MASK)                == OPCODE_CALL         // 1001 010k kkkk 111k kkkk kkkk kkkk kkkk 
            || (instruction & LDS_MASK)             == OPCODE_LDS          // 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
            || (instruction & STS_MASK)             == OPCODE_STS;         // 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
}


// 6 bit offset q has to be inserted in the opcode like this:
// 00q0 qq00 0000 0qqq
#define makeLDDSTDoffset(offset) ( \
               ((offset) & 0x07) \
            + (((offset) & 0x18) << 7) \
            + (((offset) & 0x20) << 8))
// STD                                  10q0 qq1r rrrr yqqq, with r=source register, q=offset from Y or Z, y=1 for Y 0 for Z
void emit_STD(uint8_t reg, uint8_t xy, uint8_t offset) {
    emit ((OPCODE_STD \
             + ((reg) << 4) \
             + ((xy) << 3) \
             + makeLDDSTDoffset(offset)));
}
// LDD                                  10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
void emit_LDD(uint8_t reg, uint8_t xy, uint8_t offset) {
	emit ((OPCODE_LDD \
             + ((reg) << 4) \
             + ((xy) << 3) \
             + makeLDDSTDoffset(offset)));
}

// LDI                                  1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
void emit_LDI(uint8_t reg, uint8_t constant) {
	emit ((OPCODE_LDI \
             + (((reg) - 16) << 4) \
             + makeLDIconstant(constant)));
}




//                                      0000 00rd dddd rrrr, with d=dest register, r=source register
uint16_t asm_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg) {
	return (((opcode) + ((destreg) << 4) + makeSourceRegister(srcreg)));
}
void emit_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg) {
	emit (asm_opcodeWithSrcAndDestRegOperand(opcode, destreg, srcreg));
}
//                                      0000 000d dddd 0000
uint16_t asm_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg) {
	return (((opcode) + ((reg) << 4)));
}
void emit_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg) {
	emit (asm_opcodeWithSingleRegOperand(opcode, reg));
}




//                                      0000 00kk kkkk k000, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
void emit_BRANCH(uint16_t opcode, uint8_t offset) {
	emit (opcode + makeBranchOffset(((offset)/2)));
}

// ADIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
void emit_ADIW(uint8_t reg, uint8_t constant) {
	emit ((OPCODE_ADIW \
			 + ((((reg)-24)/2)<<4) \
			 + ((constant) & 0x0F) \
             + (((constant) & 0x30) << 2)));
}

