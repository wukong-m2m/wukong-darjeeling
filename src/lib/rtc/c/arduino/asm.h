#define R0        0
#define R1        1
#define ZERO_REG  1
#define R2        2
#define R3        3
#define R4        4
#define R5        5
#define R6        6
#define R7        7
#define R8        8
#define R9        9
#define R10      10
#define R11      11
#define R12      12
#define R13      13
#define R14      14
#define R15      15
#define R16      16
#define R17      17
#define R18      18
#define R19      19
#define R20      20
#define R21      21
#define R22      22
#define R23      23
#define R24      24
#define R25      25
#define R26      26
#define R27      27
#define R28      28
#define R29      29
#define R30      30
#define R31      31

#define RX       26
#define RY       28
#define RZ       30
#define RXL      (RX)
#define RYL      (RY)
#define RZL      (RZ)
#define RXH      (RX+1)
#define RYH      (RY+1)
#define RZH      (RZ+1)

#define Y  1
#define Z  0 

#define SPaddress_L 0x5D
#define SPaddress_H 0x5E

bool rtc_is_double_word_instruction(uint16_t instruction);

// 6 bit offset q has to be inserted in the opcode like this:
// 00q0 qq00 0000 0qqq
#define makeLDDSTDoffset(offset) ( \
               ((offset) & 0x07) \
            + (((offset) & 0x18) << 7) \
            + (((offset) & 0x20) << 8))

// 0000 00kk kkkk k000
#define makeBranchOffset(offset) ( \
                ((offset) & 0x7F) << 3)


// 0000 KKKK 0000 KKKK
#define makeLDIconstant(constant) ( \
               ((constant) & 0x0F) \
            + (((constant) & 0xF0) << 4))
#define makeSBCIconstant(constant) makeLDIconstant(constant)
#define makeSUBIconstant(constant) makeLDIconstant(constant)

// 0000 0000 KK00 KKKK
#define makeADIWconstant(constant) ( \
               ((constant) & 0x0F) \
            + (((constant) & 0x30) << 2))

// 0000 00r0 0000 rrrr, with d=dest register, r=source register
#define makeSourceRegister(src_register) ( \
               ((src_register) & 0x0F) \
            + (((src_register) & 0x10) << 5))

// 0000 0AA0 0000 AAAA
#define makeINaddress(address) ( \
               ((address) & 0x0F) \
            + (((address) & 0x30) << 5))

#define opcodeWithSingleRegOperand(opcode, reg)                 ((opcode) + ((reg) << 4))
#define opcodeWithSrcAndDestRegOperand(opcode, destreg, srcreg) ((opcode) + ((destreg) << 4) + makeSourceRegister(srcreg))


// ADC                                  0001 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_ADC                      0x1C00
#define asm_ADC(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_ADC, destreg, srcreg)

// ADD                                  0000 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_ADD                      0x0C00
#define asm_ADD(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_ADD, destreg, srcreg)

// ADIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
#define OPCODE_ADIW                     0x9600
#define asm_ADIW(reg, constant)         (OPCODE_ADIW + ((((reg)-24)/2)<<4) + makeADIWconstant(constant))

// AND                                  0010 00rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_AND                      0x2000
#define asm_AND(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_AND, destreg, srcreg)

// ASR                                  1001 010d dddd 0101
#define OPCODE_ASR                      0x9405
#define asm_ASR(reg)                    opcodeWithSingleRegOperand(OPCODE_ASR, reg)

// BREAK                                1001 0101 1001 1000
#define OPCODE_BREAK                    0x9598
#define asm_BREAK(reg)                  OPCODE_BREAK


// BREQ                                 1111 00kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BREQ                     0xF001
#define asm_BREQ(offset)                (OPCODE_BREQ + makeBranchOffset(((offset)/2)))

// BRGE                                 1111 01kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRGE                     0xF404
#define asm_BRGE(offset)                (OPCODE_BRGE + makeBranchOffset(((offset)/2)))

// BRLT                                 1111 00kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRLT                     0xF004
#define asm_BRLT(offset)                (OPCODE_BRLT + makeBranchOffset(((offset)/2)))

// BRNE                                 1111 01kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRNE                     0xF401
#define asm_BRNE(offset)                (OPCODE_BRNE + makeBranchOffset(((offset)/2)))

// BRPL                                 1111 01kk kkkk k010
#define OPCODE_BRPL                     0xF402
#define asm_BRPL(offset)                (OPCODE_BRPL + makeBranchOffset(((offset)/2)))

// CALL                                 1001 010k kkkk 111k
//                                      kkkk kkkk kkkk kkkk
// TODO: support addresses > 128K
#define OPCODE_CALL                     0x940E
#define asm_CALL1(address)              OPCODE_CALL
#define asm_CALL2(address)              (address)

// CLR
#define asm_CLR(destreg)                asm_EOR(destreg, destreg)

// COM                                  1001 010d dddd 0000, with d=dest register
#define OPCODE_COM                      0x9400
#define asm_COM(reg)                    opcodeWithSingleRegOperand(OPCODE_COM, reg)

// CP                                   0001 01rd dddd rrrr, with r,d=the registers to compare
#define OPCODE_CP                       0x1400
#define asm_CP(destreg, srcreg)         opcodeWithSrcAndDestRegOperand(OPCODE_CP, destreg, srcreg)

// CPC                                  0000 01rd dddd rrrr, with r,d=the registers to compare
#define OPCODE_CPC                      0x0400
#define asm_CPC(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_CPC, destreg, srcreg)

// DEC                                  1001 010d dddd 1010
#define OPCODE_DEC                      0x940A
#define asm_DEC(reg)                    opcodeWithSingleRegOperand(OPCODE_DEC, reg)

// EOR                                  0010 01rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_EOR                      0x2400
#define asm_EOR(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_EOR, destreg, srcreg)

// IJMP                                 1001 0100 0000 1001
#define OPCODE_IJMP                     0x9409
#define asm_IJMP()                      OPCODE_IJMP

// IN                                   1011 0AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
#define OPCODE_IN                       0xB000
#define asm_IN(destreg, address)        (OPCODE_IN \
                                         + ((destreg) << 4) \
                                         + makeINaddress(address))

// INC                                  1001 010d dddd 0011, with d=dest register
#define OPCODE_INC                      0x9403
#define asm_INC(reg)                    opcodeWithSingleRegOperand(OPCODE_INC, reg)

// JMP                                  1001 010k kkkk 110k
//                                      kkkk kkkk kkkk kkkk, with k the address in WORDS, not bytes. PC <- k
// TODO: support addresses > 128K
#define SIZEOF_JMP                      4
#define OPCODE_JMP                      0x940C
#define asm_JMP1(address)               OPCODE_JMP
#define asm_JMP2(address)               (address/2)

// LD Rd, -X                            1001 000d dddd 1110, with d=dest register
#define OPCODE_LD_DECX                  0x900E
#define asm_LD_DECX(reg)                opcodeWithSingleRegOperand(OPCODE_LD_DECX, reg)

// LD Rd, X+                            1001 000d dddd 1101
#define OPCODE_LD_XINC                  0x900D
#define asm_LD_XINC(reg)                opcodeWithSingleRegOperand(OPCODE_LD_XINC, reg)

// LD Rd, Z                             1000 000d dddd 0000
#define OPCODE_LD_Z                     0x8000
#define asm_LD_Z(reg)                   opcodeWithSingleRegOperand(OPCODE_LD_Z, reg)

// LD Rd, Z+                            1001 000d dddd 0001
#define OPCODE_LD_ZINC                  0x9001
#define asm_LD_ZINC(reg)                opcodeWithSingleRegOperand(OPCODE_LD_ZINC, reg)

// LDD                                  10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
#define OPCODE_LDD                      0x8000
#define asm_LDD(reg, xy, offset)        (OPCODE_LDD \
                                         + ((reg) << 4) \
                                         + ((xy) << 3) \
                                         + makeLDDSTDoffset(offset))

// LDI                                  1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
#define OPCODE_LDI                      0xE000
#define asm_LDI(reg, constant)          (OPCODE_LDI \
                                         + (((reg) - 16) << 4) \
                                         + makeLDIconstant(constant))

// LDS                                  1001 000d dddd 0000
//                                      kkkk kkkk kkkk kkkk
#define OPCODE_LDS                      0x9000
#define asm_LDS1(reg, address)          opcodeWithSingleRegOperand(OPCODE_LDS, reg)
#define asm_LDS2(reg, address)          (address)

// LSL
#define asm_LSL(destreg)                asm_ADD(destreg, destreg)


// LSR                                  1001 010d dddd 0110
#define OPCODE_LSR                      0x9406
#define asm_LSR(reg)                    opcodeWithSingleRegOperand(OPCODE_LSR, reg)

// MOV                                  0010 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_MOV                      0x2C00
#define asm_MOV(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_MOV, destreg, srcreg)

// MOVW                                 0000 0001 dddd rrrr, with d=dest register/2, r=source register/2
#define OPCODE_MOVW                     0x0100
#define asm_MOVW(destreg, srcreg)       opcodeWithSrcAndDestRegOperand(OPCODE_MOVW, (destreg/2), (srcreg/2))

// MUL                                  1001 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_MUL                      0x9C00
#define asm_MUL(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_MUL, destreg, srcreg)

// NOP                                  0000 0000 0000 0000
#define OPCODE_NOP                      0x0000
#define asm_NOP()                       OPCODE_NOP

// OR                                   0010 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_OR                       0x2800
#define asm_OR(destreg, srcreg)         opcodeWithSrcAndDestRegOperand(OPCODE_OR, destreg, srcreg)

// PUSH                                 1001 001d dddd 1111, with d=source register
#define OPCODE_PUSH                     0x920F
#define asm_PUSH(reg)                   opcodeWithSingleRegOperand(OPCODE_PUSH, reg)

// POP                                  1001 000d dddd 1111
#define OPCODE_POP                      0x900F
#define asm_POP(reg)                    opcodeWithSingleRegOperand(OPCODE_POP, reg)

// RCALL                                1101 kkkk kkkk kkkk, with k relative in words, not bytes. PC <- PC + k + 1
#define OPCODE_RCALL                    0xD000
#define asm_RCALL(offset)               (OPCODE_RCALL + offset)

// RET                                  1001 0101 0000 1000
#define OPCODE_RET                      0x9508
#define asm_RET()                       OPCODE_RET

// RJMP                                 1100 kkkk kkkk kkkk, with k the signed offset to jump to, in WORDS, not bytes. PC <- PC + k + 1
#define SIZEOF_RJMP                     2
#define OPCODE_RJMP                     0xC000
#define asm_RJMP(offset)                (OPCODE_RJMP + (((offset)/2) & 0xFFF))

// ROL
#define asm_ROL(reg)                    asm_ADC(reg, reg)

// ROR                                  1001 010d dddd 0111
#define OPCODE_ROR                      0x9407
#define asm_ROR(reg)                    opcodeWithSingleRegOperand(OPCODE_ROR, reg)

// SBC                                  0000 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_SBC                      0x0800
#define asm_SBC(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_SBC, destreg, srcreg)

// SBCI                                 0100 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
#define OPCODE_SBCI                     0x4000
#define asm_SBCI(reg, constant)         (OPCODE_SBCI \
                                         + (((reg) - 16) << 4) \
                                         + makeSBCIconstant(constant))

// SBRC                                 1111 110r rrrr 0bbb, with r=a register and b=the bit to test
#define OPCODE_SBRC                     0xFC00
#define asm_SBRC(reg, bit)              (OPCODE_SBRC + (reg << 4) + bit)

// SBRS                                 1111 111r rrrr 0bbb, with r=a register and b=the bit to test
#define OPCODE_SBRS                     0xFE00
#define asm_SBRS(reg, bit)              (OPCODE_SBRS + (reg << 4) + bit)

// ST -X, Rs                            1001 001r rrrr 1110, with r=the register to store
#define OPCODE_ST_DECX                  0x920E
#define asm_ST_DECX(reg)                opcodeWithSingleRegOperand(OPCODE_ST_DECX, reg)

// ST X+, Rs                            1001 001r rrrr 1101, with r=the register to store
#define OPCODE_ST_XINC                  0x920D
#define asm_ST_XINC(reg)                opcodeWithSingleRegOperand(OPCODE_ST_XINC, reg)

// ST Z, Rs                             1000 001r rrrr 0000, with r=the register to store
#define OPCODE_ST_Z                     0x8200
#define asm_ST_Z(reg)                   opcodeWithSingleRegOperand(OPCODE_ST_Z, reg)

// ST Z+, Rs                            1001 001r rrrr 0001, with r=the register to store
#define OPCODE_ST_ZINC                  0x9201
#define asm_ST_ZINC(reg)                opcodeWithSingleRegOperand(OPCODE_ST_ZINC, reg)

// STD                                  10q0 qq1r rrrr yqqq, with r=source register, q=offset from Y or Z, y=1 for Y 0 for Z
#define OPCODE_STD                      0x8200
#define asm_STD(reg, xy, offset)        (OPCODE_STD \
                                         + ((reg) << 4) \
                                         + ((xy) << 3) \
                                         + makeLDDSTDoffset(offset))

// STS                                  1001 001d dddd 0000
//                                      kkkk kkkk kkkk kkkk
#define OPCODE_STS                      0x9200
#define asm_STS1(address, reg)          opcodeWithSingleRegOperand(OPCODE_STS, reg)
#define asm_STS2(address, reg)          (address)

// SUB                                  0001 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_SUB                      0x1800
#define asm_SUB(destreg, srcreg)        opcodeWithSrcAndDestRegOperand(OPCODE_SUB, destreg, srcreg)

// SUBI                                 0101 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
#define OPCODE_SUBI                     0x5000
#define asm_SUBI(reg, constant)         (OPCODE_SUBI \
                                         + (((reg) - 16) << 4) \
                                         + makeSUBIconstant(constant))


// emit functions
#define emit_ADC(destreg, srcreg)       emit ( asm_ADC(destreg, srcreg) )
#define emit_ADD(destreg, srcreg)       emit ( asm_ADD(destreg, srcreg) )
#define emit_ADIW(reg, constant)        emit ( asm_ADIW(reg, constant) )
#define emit_AND(destreg, srcreg)       emit ( asm_AND(destreg, srcreg) )
#define emit_ASR(reg)                   emit ( asm_ASR(reg) )
#define emit_BREAK()                    emit ( asm_BREAK() )
#define emit_BREQ(offset)               emit ( asm_BREQ(offset) )
#define emit_BRGE(offset)               emit ( asm_BRGE(offset) )
#define emit_BRLT(offset)               emit ( asm_BRLT(offset) )
#define emit_BRNE(offset)               emit ( asm_BRNE(offset) )
#define emit_BRPL(offset)               emit ( asm_BRPL(offset) )
#define emit_CLR(destreg)               emit ( asm_CLR(destreg) )
#define emit_COM(reg)                   emit ( asm_COM(reg) )
#define emit_CP(destreg, srcreg)        emit ( asm_CP(destreg, srcreg) )
#define emit_CPC(destreg, srcreg)       emit ( asm_CPC(destreg, srcreg) )
#define emit_DEC(reg)                   emit ( asm_DEC(reg) )
#define emit_EOR(destreg, srcreg)       emit ( asm_EOR(destreg, srcreg) )
#define emit_IJMP()                     emit ( asm_IJMP() )
#define emit_IN(destreg, address)       emit ( asm_IN(destreg, address) )
#define emit_INC(reg)                   emit ( asm_INC(reg) )
#define emit_2_JMP(address)             emit2( asm_JMP1(address) , asm_JMP2(address) )
#define emit_LD_DECX(reg)               emit ( asm_LD_DECX(reg) )
#define emit_LD_XINC(reg)               emit ( asm_LD_XINC(reg) )
#define emit_LD_Z(reg)                  emit ( asm_LD_Z(reg) )
#define emit_LD_ZINC(reg)               emit ( asm_LD_ZINC(reg) )
#define emit_LDD(reg, xy, offset)       emit ( asm_LDD(reg, xy, offset) )
#define emit_LDI(reg, constant)         emit ( asm_LDI(reg, constant) )
#define emit_2_LDS(reg, address)        emit2( asm_LDS1(reg, address) , asm_LDS2(reg, address) )
#define emit_LSL(destreg)               emit ( asm_LSL(destreg) )
#define emit_LSR(reg)                   emit ( asm_LSR(reg) )
#define emit_MOV(destreg, srcreg)       emit ( asm_MOV(destreg, srcreg) )
#define emit_MOVW(destreg, srcreg)      emit ( asm_MOVW(destreg, srcreg) )
#define emit_MUL(destreg, srcreg)       emit ( asm_MUL(destreg, srcreg) )
#define emit_NOP()                      emit ( asm_NOP() )
#define emit_OR(destreg, srcreg)        emit ( asm_OR(destreg, srcreg) )
#define emit_PUSH(reg)                  emit ( asm_PUSH(reg) )
#define emit_POP(reg)                   emit ( asm_POP(reg) )
#define emit_RCALL(offset)              emit ( asm_RCALL(offset) )
#define emit_RET()                      emit ( asm_RET()  )
#define emit_RJMP(offset)               emit ( asm_RJMP(offset) )
#define emit_ROL(reg)                   emit ( asm_ROL(reg) )
#define emit_ROR(reg)                   emit ( asm_ROR(reg) )
#define emit_SBC(destreg, srcreg)       emit ( asm_SBC(destreg, srcreg) )
#define emit_SBCI(reg, constant)        emit ( asm_SBCI(reg, constant) )
#define emit_SBRC(reg, bit)             emit ( asm_SBRC(reg, bit) )
#define emit_SBRS(reg, bit)             emit ( asm_SBRS(reg, bit) )
#define emit_ST_DECX(reg)               emit ( asm_ST_DECX(reg) )
#define emit_ST_XINC(reg)               emit ( asm_ST_XINC(reg) )
#define emit_ST_Z(reg)                  emit ( asm_ST_Z(reg) )
#define emit_ST_ZINC(reg)               emit ( asm_ST_ZINC(reg) )
#define emit_STD(reg, xy, offset)       emit ( asm_STD(reg, xy, offset) )
#define emit_2_STS(address, reg)        emit2( asm_STS1(address, reg) , asm_STS2(address, reg) )
#define emit_SUB(destreg, srcreg)       emit ( asm_SUB(destreg, srcreg) )
#define emit_SUBI(reg, constant)        emit ( asm_SUBI(reg, constant) )

