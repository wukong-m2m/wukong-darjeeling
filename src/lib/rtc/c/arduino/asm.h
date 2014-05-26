#define R0			0
#define R1			1
#define R2			2
#define R3			3
#define R4			4
#define R5			5
#define R6			6
#define R7			7
#define R8			8
#define R9			9
#define R10			10
#define R11			11
#define R12			12
#define R13			13
#define R14			14
#define R15			15
#define R16			16
#define R17			17
#define R18			18
#define R19			19
#define R20			20
#define R21			21
#define R22			22
#define R23			23
#define R24			24
#define R25			25
#define R26			26
#define R27			27
#define R28			28
#define R29			29
#define R30			30
#define R31			31

#define Y			1
#define Z			0



// LDD  	            10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
#define OPCODE_LDD 		0x8000

// LD Rd	,X+         1001 000d dddd 1101
#define OPCODE_LDXINC	0x900D

// PUSH 	            1001 001d dddd 1111, with d=source register
#define OPCODE_PUSH		0x920F

// POP  	            1001 000d dddd 1111
#define OPCODE_POP		0x900F

// MOVW               0000 0001 dddd rrrr, with d=dest register/2, r=source register/2
#define OPCODE_MOVW   0x0100

// LDI  	            1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
#define OPCODE_LDI		0xE000

// ADD  	            0000 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_ADD		0x0C00

// ADC  	            0001 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_ADC		0x1C00

// SUB                0001 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_SUB    0x1800

// MUL                1001 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_MUL    0x9C00

// SBC                0000 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_SBC    0x0800

// RET  	            1001 0101 0000 1000
#define OPCODE_RET		0x9508


// 6 bit offset q has to be inserted in the opcode like this:
// 00q0 qq00 0000 0qqq
#define makeLDDoffset(offset) ( \
               ((offset) & 0x07) \
            + (((offset) & 0x18) << 7) \
            + (((offset) & 0x20) << 8))

// 0000 KKKK 0000 KKKK
#define makeLDIconstant(constant) ( \
               ((constant) & 0x0F) \
            + (((constant) & 0xF0) << 4))

// 0000 00r0 0000 rrrr, with d=dest register, r=source register
#define makeSourceRegister(src_register) ( \
               ((src_register) & 0x0F) \
            + (((src_register) & 0x10) << 5))


#define opcodeWithSingleRegOperand(opcode, reg)                 ((opcode) + ((reg) << 4))
#define opcodeWithSrcAndDestRegOperand(opcode, destreg, srcreg) ((opcode) + ((destreg) << 4) + makeSourceRegister(srcreg))


#define asm_PUSH(reg)                   opcodeWithSingleRegOperand(OPCODE_PUSH, reg)
#define asm_POP(reg)                    opcodeWithSingleRegOperand(OPCODE_POP, reg)
#define asm_LDXINC(reg)                 opcodeWithSingleRegOperand(OPCODE_LDXINC, reg)
#define asm_LDD(reg, xy, offset)        (OPCODE_LDD \
                                     + ((reg) << 4) \
                                     + ((xy) << 3) \
                                     + makeLDDoffset(offset))
#define asm_LDI(reg, constant)          (OPCODE_LDI \
                                     + (((reg) - 16) << 4) \
                                     + makeLDIconstant(constant))
#define asm_ADD(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_ADD, destreg, srcreg)
#define asm_ADC(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_ADC, destreg, srcreg)
#define asm_MUL(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_MUL, destreg, srcreg)
#define asm_SUB(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_SUB, destreg, srcreg)
#define asm_SBC(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_SBC, destreg, srcreg)
#define asm_MOVW(destreg, srcreg)       opcodeWithSrcAndDestRegOperand(OPCODE_MOVW, (destreg/2), (srcreg/2))
#define asm_RET                         OPCODE_RET 



