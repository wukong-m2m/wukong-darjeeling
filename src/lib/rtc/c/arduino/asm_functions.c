#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "panic.h"
#include "asm.h"
#include "rtc_emit.h"

#define ASM_GUARDS // Define this for debugging to add some checks to the assembler.
#ifdef ASM_GUARDS
void asm_guard_assert(bool condition) {
    if (!condition) {
        while(true) { dj_panic(DJ_PANIC_AOT_ASM_ERROR); }
    }
}
void asm_guard_check_regs(uint16_t opcode, uint8_t reg1, uint8_t reg2) {
    switch(opcode) {
        case OPCODE_ADIW: // r24, X(r26), Y(r28), Z(r30)
            asm_guard_assert(reg1==R24 || reg1==R26 || reg1==R28 || reg1==R30);
        break;
        case OPCODE_MOVW: // only even reg pairs
            asm_guard_assert((reg1%2) == 0 && (reg2%2) == 0);
        break;
        case OPCODE_LDI : // r16-31 // 0xE000
        case OPCODE_SBCI: // r16-31 // 0x4000
        case OPCODE_SUBI: // r16-31 // 0x5000
            asm_guard_assert(R16 <= reg1 && reg1 <= R31);
        break;
        default:
            asm_guard_assert(false);
    }
}
#else // RTC_ASM_GUARDS
#define asm_guard_check_regs(opcode, reg1, reg2)
#endif

bool rtc_is_double_word_instruction(uint16_t instruction) {
    const uint16_t CALL_MASK            = 0xFE0E;
    const uint16_t LDS_MASK             = 0xFE0F;
    const uint16_t STS_MASK             = 0xFE0F;

    return (instruction & CALL_MASK)                == OPCODE_CALL         // 1001 010k kkkk 111k kkkk kkkk kkkk kkkk 
            || (instruction & LDS_MASK)             == OPCODE_LDS          // 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
            || (instruction & STS_MASK)             == OPCODE_STS;         // 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
}

//                                      0000 000d dddd 0000
uint16_t asm_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg) {
	return (((opcode) + ((reg) << 4)));
}
void emit_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg) {
	emit (asm_opcodeWithSingleRegOperand(opcode, reg));
}
//                                      0000 00rd dddd rrrr, with d=dest register, r=source register
uint16_t asm_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg) {
    return (((opcode) + ((destreg) << 4) + makeSourceRegister(srcreg)));
}
void emit_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg) {
    emit (asm_opcodeWithSrcAndDestRegOperand(opcode, destreg, srcreg));
}


// ADIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
void emit_ADIW(uint8_t reg, uint8_t constant) {
    asm_guard_check_regs(OPCODE_ADIW, reg, 0);
	emit ((OPCODE_ADIW
			 + ((((reg)-24)/2)<<4)
			 + ((constant) & 0x0F)
             + (((constant) & 0x30) << 2)));
}
//                                      0000 00kk kkkk k000, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
void emit_BRANCH(uint16_t opcode, uint8_t offset) {
    emit (opcode + makeBranchOffset(((offset)/2)));
}

uint16_t emit_ADIW_if_necessary_to_bring_offset_in_range(uint8_t reg, uint16_t offset) {
    // LDD/STD can accept an offset up to 63, but since this may be the first LDD/STD of a
    // 32 bit int, we add a margin of 3 more bytes. Maybe we'll emit an ADIW too many in some
    // cases, but it keeps the rest of the code a little bit smaller and cleaner.
    while (offset > 60) {
        emit_ADIW(reg, 63);
        offset -= 63;
    }
    return offset;
}

// 6 bit offset q has to be inserted in the opcode like this:
// 00q0 qq00 0000 0qqq
#define makeLDDSTDoffset(offset) ( \
               ((offset) & 0x07) \
            + (((offset) & 0x18) << 7) \
            + (((offset) & 0x20) << 8))
// LDD                                  10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
void emit_LDD(uint8_t reg, uint8_t yz, uint16_t offset) {
    if (offset > 63) {
        dj_panic(DJ_PANIC_AOT_ASM_ERROR_OFFSET_OUT_OF_RANGE);
    }
    emit (OPCODE_LDD
             + ((reg) << 4)
             + ((yz) << 3)
             + makeLDDSTDoffset(offset));
}
// STD                                  10q0 qq1r rrrr yqqq, with r=source register, q=offset from Y or Z, y=1 for Y 0 for Z
void emit_STD(uint8_t reg, uint8_t yz, uint16_t offset) {
    if (offset > 63) {
        dj_panic(DJ_PANIC_AOT_ASM_ERROR_OFFSET_OUT_OF_RANGE);
    }
    emit ((OPCODE_STD
             + ((reg) << 4)
             + ((yz) << 3)
             + makeLDDSTDoffset(offset)));
}


// LDI                                  1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
// SBCI                                 0100 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
// SUBI                                 0101 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
void emit_LDI_SBCI_SUBI(uint16_t opcode, uint8_t reg, uint8_t constant) {
    asm_guard_check_regs(opcode, reg, 0);
    uint16_t encoded_constant = (constant & 0x0F) + ((constant & 0xF0) << 4); // 0000 KKKK 0000 KKKK
    emit (opcode
             + (((reg) - 16) << 4)
             + encoded_constant);

}
void emit_LDI(uint8_t reg, uint8_t constant) { emit_LDI_SBCI_SUBI(OPCODE_LDI, reg, constant); }
void emit_SBCI(uint8_t reg, uint8_t constant) { emit_LDI_SBCI_SUBI(OPCODE_SBCI, reg, constant); }
void emit_SUBI(uint8_t reg, uint8_t constant) { emit_LDI_SBCI_SUBI(OPCODE_SUBI, reg, constant); }

uint16_t asm_MOVW(uint8_t destreg, uint8_t srcreg) {
    asm_guard_check_regs(OPCODE_MOVW, destreg, srcreg);
    return asm_opcodeWithSrcAndDestRegOperand(OPCODE_MOVW, (destreg/2), (srcreg/2));
}
void emit_MOVW(uint8_t destreg, uint8_t srcreg) {
    asm_guard_check_regs(OPCODE_MOVW, destreg, srcreg);
    emit_opcodeWithSrcAndDestRegOperand(OPCODE_MOVW, (destreg/2), (srcreg/2));
}

