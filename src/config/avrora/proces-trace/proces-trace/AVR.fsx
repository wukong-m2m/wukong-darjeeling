//                                      1111 11rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_DOUBLE_REG_OPERAND             = 0xFC00
//                                      1111 111d dddd 1111
[<Literal>]
let MASK_SINGLE_REG_OPERAND             = 0xFE0F
//                                      1111 11kk kkkk k111, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
[<Literal>]
let MASK_BRANCH                         = 0xFC07

     
// ADC                                  0001 11rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_ADC                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_ADC                          = 0x1C00

// ADD                                  0000 11rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_ADD                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_ADD                          = 0x0C00

// ADIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
[<Literal>]
let MASK_ADIW                           = 0xFF00
[<Literal>]
let OPCODE_ADIW                         = 0x9600

// AND                                  0010 00rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_AND                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_AND                          = 0x2000

// ASR                                  1001 010d dddd 0101
[<Literal>]
let MASK_ASR                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_ASR                          = 0x9405

// BREAK                                1001 0101 1001 1000
[<Literal>]
let MASK_BREAK                          = 0xFFFF
[<Literal>]
let OPCODE_BREAK                        = 0x9598

// BREQ                                 1111 00kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
[<Literal>]
let MASK_BREQ                           = MASK_BRANCH
[<Literal>]
let OPCODE_BREQ                         = 0xF001

// BRGE                                 1111 01kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
[<Literal>]
let MASK_BRGE                           = MASK_BRANCH
[<Literal>]
let OPCODE_BRGE                         = 0xF404

// BRLT                                 1111 00kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
[<Literal>]
let MASK_BRLT                           = MASK_BRANCH
[<Literal>]
let OPCODE_BRLT                         = 0xF004

// BRNE                                 1111 01kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
[<Literal>]
let MASK_BRNE                           = MASK_BRANCH
[<Literal>]
let OPCODE_BRNE                         = 0xF401

// BRPL                                 1111 01kk kkkk k010
[<Literal>]
let MASK_BRPL                           = MASK_BRANCH
[<Literal>]
let OPCODE_BRPL                         = 0xF402

// CALL                                 1001 010k kkkk 111k
//                                      kkkk kkkk kkkk kkkk
[<Literal>]
let MASK_CALL                           = 0xFE0E
[<Literal>]
let OPCODE_CALL                         = 0x940E

// COM                                  1001 010d dddd 0000, with d=dest register
[<Literal>]
let MASK_COM                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_COM                          = 0x9400

// CP                                   0001 01rd dddd rrrr, with r,d=the registers to compare
[<Literal>]
let MASK_CP                             = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_CP                           = 0x1400

// CPC                                  0000 01rd dddd rrrr, with r,d=the registers to compare
[<Literal>]
let MASK_CPC                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_CPC                          = 0x0400

// DEC                                  1001 010d dddd 1010
[<Literal>]
let MASK_DEC                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_DEC                          = 0x940A

// EOR                                  0010 01rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_EOR                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_EOR                          = 0x2400

// IJMP                                 1001 0100 0000 1001
[<Literal>]
let MASK_IJMP                           = 0xFFFF
[<Literal>]
let OPCODE_IJMP                         = 0x9409

// IN                                   1011 0AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
[<Literal>]
let MASK_IN                             = 0xF800
[<Literal>]
let OPCODE_IN                           = 0xB000

// INC                                  1001 010d dddd 0011, with d=dest register
[<Literal>]
let MASK_INC                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_INC                          = 0x9403

// JMP                                  1001 010k kkkk 110k
//                                      kkkk kkkk kkkk kkkk, with k the address in WORDS, not bytes. PC <- k
// TODO: support addresses > 128K
[<Literal>]
let MASK_JMP                            = 0xFE0E
[<Literal>]
let OPCODE_JMP                          = 0x940C

// LD Rd, -X                            1001 000d dddd 1110, with d=dest register
[<Literal>]
let MASK_LD_DECX                        = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_LD_DECX                      = 0x900E

// LD Rd, X+                            1001 000d dddd 1101
[<Literal>]
let MASK_LD_XINC                        = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_LD_XINC                      = 0x900D

// LD Rd, Z                             1000 000d dddd 0000
[<Literal>]
let MASK_LD_Z                           = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_LD_Z                         = 0x8000

// LD Rd, Z+                            1001 000d dddd 0001
[<Literal>]
let MASK_LD_ZINC                        = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_LD_ZINC                      = 0x9001

// LDD                                  10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
[<Literal>]
let MASK_LDD                            = 0xD200
[<Literal>]
let OPCODE_LDD                          = 0x8000

// LDI                                  1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
[<Literal>]
let MASK_LDI                            = 0xF000
[<Literal>]
let OPCODE_LDI                          = 0xE000

// LDS                                  1001 000d dddd 0000
//                                      kkkk kkkk kkkk kkkk
[<Literal>]
let MASK_LDS                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_LDS                          = 0x9000

// LSR                                  1001 010d dddd 0110
[<Literal>]
let MASK_LSR                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_LSR                          = 0x9406

// MOV                                  0010 11rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_MOV                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_MOV                          = 0x2C00

// MOVW                                 0000 0001 dddd rrrr, with d=dest register/2, r=source register/2
[<Literal>]
let MASK_MOVW                           = 0xFF00
[<Literal>]
let OPCODE_MOVW                         = 0x0100

// MUL                                  1001 11rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_MUL                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_MUL                          = 0x9C00

// NOP                                  0000 0000 0000 0000
[<Literal>]
let MASK_NOP                            = 0xFFFF
[<Literal>]
let OPCODE_NOP                          = 0x0000

// OR                                   0010 10rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_OR                             = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_OR                           = 0x2800

// PUSH                                 1001 001d dddd 1111, with d=source register
[<Literal>]
let MASK_PUSH                           = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_PUSH                         = 0x920F

// POP                                  1001 000d dddd 1111
[<Literal>]
let MASK_POP                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_POP                          = 0x900F

// RCALL                                1101 kkkk kkkk kkkk, with k relative in words, not bytes. PC <- PC + k + 1
[<Literal>]
let MASK_RCALL                          = 0xF000
[<Literal>]
let OPCODE_RCALL                        = 0xD000

// RET                                  1001 0101 0000 1000
[<Literal>]
let MASK_RET                            = 0xFFFF
[<Literal>]
let OPCODE_RET                          = 0x9508

// RJMP                                 1100 kkkk kkkk kkkk, with k the signed offset to jump to, in WORDS, not bytes. PC <- PC + k + 1
[<Literal>]
let MASK_RJMP                           = 0xF000
[<Literal>]
let OPCODE_RJMP                         = 0xC000

// ROR                                  1001 010d dddd 0111
[<Literal>]
let MASK_ROR                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_ROR                          = 0x9407

// SBC                                  0000 10rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_SBC                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_SBC                          = 0x0800

// SBCI                                 0100 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
[<Literal>]
let MASK_SBCI                           = 0xF000
[<Literal>]
let OPCODE_SBCI                         = 0x4000

// SBRC                                 1111 110r rrrr 0bbb, with r=a register and b=the bit to test
[<Literal>]
let MASK_SBRC                           = 0xFE08
[<Literal>]
let OPCODE_SBRC                         = 0xFC00

// SBRS                                 1111 111r rrrr 0bbb, with r=a register and b=the bit to test
[<Literal>]
let MASK_SBRS                           = 0xFE08
[<Literal>]
let OPCODE_SBRS                         = 0xFE00

// ST -X, Rs                            1001 001r rrrr 1110, with r=the register to store
[<Literal>]
let MASK_ST_DECX                        = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_ST_DECX                      = 0x920E

// ST X+, Rs                            1001 001r rrrr 1101, with r=the register to store
[<Literal>]
let MASK_ST_XINC                        = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_ST_XINC                      = 0x920D

// ST Z, Rs                             1000 001r rrrr 0000, with r=the register to store
[<Literal>]
let MASK_ST_Z                           = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_ST_Z                         = 0x8200

// ST Z+, Rs                            1001 001r rrrr 0001, with r=the register to store
[<Literal>]
let MASK_ST_ZINC                        = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_ST_ZINC                      = 0x9201

// STD                                  10q0 qq1r rrrr yqqq, with r=source register, q=offset from Y or Z, y=1 for Y 0 for Z
[<Literal>]
let MASK_STD                            = 0xD200
[<Literal>]
let OPCODE_STD                          = 0x8200

// STS                                  1001 001d dddd 0000
//                                      kkkk kkkk kkkk kkkk
[<Literal>]
let MASK_STS                            = MASK_SINGLE_REG_OPERAND
[<Literal>]
let OPCODE_STS                          = 0x9200

// SUB                                  0001 10rd dddd rrrr, with d=dest register, r=source register
[<Literal>]
let MASK_SUB                            = MASK_DOUBLE_REG_OPERAND
[<Literal>]
let OPCODE_SUB                          = 0x1800

// SUBI                                 0101 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
[<Literal>]
let MASK_SUBI                           = 0xF000
[<Literal>]
let OPCODE_SUBI                         = 0x5000



let isADC = (&&&) MASK_ADC >> (=) OPCODE_ADC
let isADD = (&&&) MASK_ADD >> (=) OPCODE_ADD
let isADIW = (&&&) MASK_ADIW >> (=) OPCODE_ADIW
let isAND = (&&&) MASK_AND >> (=) OPCODE_AND
let isASR = (&&&) MASK_ASR >> (=) OPCODE_ASR
let isBREAK = (&&&) MASK_BREAK >> (=) OPCODE_BREAK
let isBREQ = (&&&) MASK_BREQ >> (=) OPCODE_BREQ
let isBRGE = (&&&) MASK_BRGE >> (=) OPCODE_BRGE
let isBRLT = (&&&) MASK_BRLT >> (=) OPCODE_BRLT
let isBRNE = (&&&) MASK_BRNE >> (=) OPCODE_BRNE
let isBRPL = (&&&) MASK_BRPL >> (=) OPCODE_BRPL
let isCALL = (&&&) MASK_CALL >> (=) OPCODE_CALL
let isCOM = (&&&) MASK_COM >> (=) OPCODE_COM
let isCP = (&&&) MASK_CP >> (=) OPCODE_CP
let isCPC = (&&&) MASK_CPC >> (=) OPCODE_CPC
let isDEC = (&&&) MASK_DEC >> (=) OPCODE_DEC
let isEOR = (&&&) MASK_EOR >> (=) OPCODE_EOR
let isIJMP = (&&&) MASK_IJMP >> (=) OPCODE_IJMP
let isIN = (&&&) MASK_IN >> (=) OPCODE_IN
let isINC = (&&&) MASK_INC >> (=) OPCODE_INC
let isJMP = (&&&) MASK_JMP >> (=) OPCODE_JMP
let isLD_DECX = (&&&) MASK_LD_DECX >> (=) OPCODE_LD_DECX
let isLD_XINC = (&&&) MASK_LD_XINC >> (=) OPCODE_LD_XINC
let isLD_Z = (&&&) MASK_LD_Z >> (=) OPCODE_LD_Z
let isLD_ZINC = (&&&) MASK_LD_ZINC >> (=) OPCODE_LD_ZINC
let isLDD = (&&&) MASK_LDD >> (=) OPCODE_LDD
let isLDI = (&&&) MASK_LDI >> (=) OPCODE_LDI
let isLDS = (&&&) MASK_LDS >> (=) OPCODE_LDS
let isLSR = (&&&) MASK_LSR >> (=) OPCODE_LSR
let isMOV = (&&&) MASK_MOV >> (=) OPCODE_MOV
let isMOVW = (&&&) MASK_MOVW >> (=) OPCODE_MOVW
let isMUL = (&&&) MASK_MUL >> (=) OPCODE_MUL
let isNOP = (&&&) MASK_NOP >> (=) OPCODE_NOP
let isOR = (&&&) MASK_OR >> (=) OPCODE_OR
let isPUSH = (&&&) MASK_PUSH >> (=) OPCODE_PUSH
let isPOP = (&&&) MASK_POP >> (=) OPCODE_POP
let isRCALL = (&&&) MASK_RCALL >> (=) OPCODE_RCALL
let isRET = (&&&) MASK_RET >> (=) OPCODE_RET
let isRJMP = (&&&) MASK_RJMP >> (=) OPCODE_RJMP
let isROR = (&&&) MASK_ROR >> (=) OPCODE_ROR
let isSBC = (&&&) MASK_SBC >> (=) OPCODE_SBC
let isSBCI = (&&&) MASK_SBCI >> (=) OPCODE_SBCI
let isSBRC = (&&&) MASK_SBRC >> (=) OPCODE_SBRC
let isSBRS = (&&&) MASK_SBRS >> (=) OPCODE_SBRS
let isST_DECX = (&&&) MASK_ST_DECX >> (=) OPCODE_ST_DECX
let isST_XINC = (&&&) MASK_ST_XINC >> (=) OPCODE_ST_XINC
let isST_Z = (&&&) MASK_ST_Z >> (=) OPCODE_ST_Z
let isST_ZINC = (&&&) MASK_ST_ZINC >> (=) OPCODE_ST_ZINC
let isSTD = (&&&) MASK_STD >> (=) OPCODE_STD
let isSTS = (&&&) MASK_STS >> (=) OPCODE_STS
let isSUB = (&&&) MASK_SUB >> (=) OPCODE_SUB
let isSUBI = (&&&) MASK_SUBI >> (=) OPCODE_SUBI
