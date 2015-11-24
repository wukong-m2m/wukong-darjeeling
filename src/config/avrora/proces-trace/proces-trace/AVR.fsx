open System

//                                      1111 11rd dddd rrrr, with d=dest register, r=source register
let MASK_DOUBLE_REG_OPERAND             = 0xFC00
//                                      1111 111d dddd 1111
let MASK_SINGLE_REG_OPERAND             = 0xFE0F
//                                      1111 11kk kkkk k111, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let MASK_BRANCH                         = 0xFC07

// avr opcode ADC                   0001 11rd dddd rrrr, with d=dest register, r=source register
let ADC                           = (0x1C00, MASK_DOUBLE_REG_OPERAND, "ADC")

// avr opcode ADD                   0000 11rd dddd rrrr, with d=dest register, r=source register
let ADD                           = (0x0C00, MASK_DOUBLE_REG_OPERAND, "ADD")

// avr opcode ADIW                  1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
let ADIW                          = (0x9600, 0xFF00, "ADIW")

// avr opcode AND                   0010 00rd dddd rrrr, with d=dest register, r=source register
let AND                           = (0x2000, MASK_DOUBLE_REG_OPERAND, "AND")

// avr opcode ANDI                  0111 KKKK dddd KKKK, with d=dest register-16, K=8 bit constant
let ANDI                          = (0x7000, 0xF000, "ANDI")

// avr opcode ASR                   1001 010d dddd 0101
let ASR                           = (0x9405, MASK_SINGLE_REG_OPERAND, "ASR")

// avr opcode BLD                   1111 100d dddd 0bbb, with d=dest register, 0<=b<=7
let BLD                           = (0xF800, 0xFE08, "BLD")

// avr opcode BREAK                 1001 0101 1001 1000
let BREAK                         = (0x9598, 0xFFFF, "BREAK")

// avr opcode BRCC                  1111 01kk kkkk k000
let BRCC                          = (0xF400, MASK_BRANCH, "BRCC")

// avr opcode BRCS                  1111 00kk kkkk k000
let BRCS                          = (0xF000, MASK_BRANCH, "BRCS")

// avr opcode BREQ                  1111 00kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BREQ                          = (0xF001, MASK_BRANCH, "BREQ")

// avr opcode BRGE                  1111 01kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRGE                          = (0xF404, MASK_BRANCH, "BRGE")

// avr opcode BRLT                  1111 00kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRLT                          = (0xF004, MASK_BRANCH, "BRLT")

// avr opcode BRNE                  1111 01kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRNE                          = (0xF401, MASK_BRANCH, "BRNE")

// avr opcode BRPL                  1111 01kk kkkk k010
let BRPL                          = (0xF402, MASK_BRANCH, "BRPL")

//                                  1001 010k kkkk 111k
// avr opcode CALL                  kkkk kkkk kkkk kkkk
let CALL                          = (0x940E, 0xFE0E, "CALL")

// avr opcode COM                   1001 010d dddd 0000, with d=dest register
let COM                           = (0x9400, MASK_SINGLE_REG_OPERAND, "COM")

// avr opcode CLI                   1001 0100 1111 1000
let CLI                           = (0x94F8, 0xFFFF, "CLI")

// avr opcode CP                    0001 01rd dddd rrrr, with r,d=the registers to compare
let CP                            = (0x1400, MASK_DOUBLE_REG_OPERAND, "CP")

// avr opcode CPC                   0000 01rd dddd rrrr, with r,d=the registers to compare
let CPC                           = (0x0400, MASK_DOUBLE_REG_OPERAND, "CPC")

// avr opcode CPI                   0011 KKKK dddd KKKK, with 16 ≤ d ≤ 31, 0≤ K ≤ 255
let CPI                           = (0x03000, 0xF000, "CPI")

// avr opcode DEC                   1001 010d dddd 1010
let DEC                           = (0x940A, MASK_SINGLE_REG_OPERAND, "DEC")

// avr opcode EOR                   0010 01rd dddd rrrr, with d=dest register, r=source register
let EOR                           = (0x2400, MASK_DOUBLE_REG_OPERAND, "EOR")

// avr opcode IJMP                  1001 0100 0000 1001
let IJMP                          = (0x9409, 0xFFFF, "IJMP")

// avr opcode IN                    1011 0AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
let IN                            = (0xB000, 0xF800, "IN")

// avr opcode INC                   1001 010d dddd 0011, with d=dest register
let INC                           = (0x9403, MASK_SINGLE_REG_OPERAND, "INC")

//                                  1001 010k kkkk 110k
// avr opcode JMP                   kkkk kkkk kkkk kkkk, with k the address in WORDS, not bytes. PC <- k
let JMP                           = (0x940C, 0xFE0E, "JMP")

// avr opcode LD Rd, X              1001 000d dddd 1100, with d=dest register
let LD_X                          = (0x900C, MASK_SINGLE_REG_OPERAND, "LD_X")

// avr opcode LD Rd, X+             1001 000d dddd 1101
let LD_XINC                       = (0x900D, MASK_SINGLE_REG_OPERAND, "LD_XINC")

// avr opcode LD Rd, -X             1001 000d dddd 1110, with d=dest register
let LD_DECX                       = (0x900E, MASK_SINGLE_REG_OPERAND, "LD_DECX")

// avr opcode LD Rd, Y              1000 000d dddd 1000, with d=dest register
let LD_Y                          = (0x8008, MASK_SINGLE_REG_OPERAND, "LD_Y")

// avr opcode LD Rd, Y+             1001 000d dddd 1001
let LD_YINC                       = (0x9009, MASK_SINGLE_REG_OPERAND, "LD_YINC")

// avr opcode LD Rd, -Y             1001 000d dddd 1010, with d=dest register
let LD_DECY                       = (0x900A, MASK_SINGLE_REG_OPERAND, "LD_DECY")

// avr opcode LDD_Y                 10q0 qq0d dddd 1qqq, with d=dest register, q=offset from Y
let LDD_Y                         = (0x8008, 0xD208, "LDD_Y")

// avr opcode LD Rd, Z              1000 000d dddd 0000, with d=dest register
let LD_Z                          = (0x8000, MASK_SINGLE_REG_OPERAND, "LD_Z")

// avr opcode LD Rd, Z+             1001 000d dddd 0001
let LD_ZINC                       = (0x9001, MASK_SINGLE_REG_OPERAND, "LD_ZINC")

// avr opcode LD Rd, -Z             1001 000d dddd 0010, with d=dest register
let LD_DECZ                       = (0x9002, MASK_SINGLE_REG_OPERAND, "LD_DECZ")

// avr opcode LDD_Z                 10q0 qq0d dddd 0qqq, with d=dest register, q=offset from Z
let LDD_Z                         = (0x8000, 0xD208, "LDD_Z")

// avr opcode LDI                   1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
let LDI                           = (0xE000, 0xF000, "LDI")

//                                  1001 000d dddd 0000
// avr opcode LDS                   kkkk kkkk kkkk kkkk
let LDS                           = (0x9000, MASK_SINGLE_REG_OPERAND, "LDS")

// avr opcode LSR                   1001 010d dddd 0110
let LSR                           = (0x9406, MASK_SINGLE_REG_OPERAND, "LSR")

// avr opcode MOV                   0010 11rd dddd rrrr, with d=dest register, r=source register
let MOV                           = (0x2C00, MASK_DOUBLE_REG_OPERAND, "MOV")

// avr opcode MOVW                  0000 0001 dddd rrrr, with d=dest register/2, r=source register/2
let MOVW                          = (0x0100, 0xFF00, "MOVW")

// avr opcode MUL                   1001 11rd dddd rrrr, with d=dest register, r=source register
let MUL                           = (0x9C00, MASK_DOUBLE_REG_OPERAND, "MUL")

// avr opcode NOP                   0000 0000 0000 0000
let NOP                           = (0x0000, 0xFFFF, "NOP")

// avr opcode OR                    0010 10rd dddd rrrr, with d=dest register, r=source register
let OR                            = (0x2800, MASK_DOUBLE_REG_OPERAND, "OR")

// avr opcode ORI                   0110 KKKK dddd KKKK, with d=dest register-16, K=8 bit constant
let ORI                           = (0x6000, 0xF000, "ORI")

// avr opcode OUT                   1011 1AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
let OUT                           = (0xB800, 0xF800, "OUT")

// avr opcode PUSH                  1001 001d dddd 1111, with d=source register
let PUSH                          = (0x920F, MASK_SINGLE_REG_OPERAND, "PUSH")

// avr opcode POP                   1001 000d dddd 1111
let POP                           = (0x900F, MASK_SINGLE_REG_OPERAND, "POP")

// avr opcode RCALL                 1101 kkkk kkkk kkkk, with k relative in words, not bytes. PC <- PC + k + 1
let RCALL                         = (0xD000, 0xF000, "RCALL")

// avr opcode RET                   1001 0101 0000 1000
let RET                           = (0x9508, 0xFFFF, "RET")

// avr opcode RJMP                  1100 kkkk kkkk kkkk, with k the signed offset to jump to, in WORDS, not bytes. PC <- PC + k + 1
let RJMP                          = (0xC000, 0xF000, "RJMP")

// avr opcode ROR                   1001 010d dddd 0111
let ROR                           = (0x9407, MASK_SINGLE_REG_OPERAND, "ROR")

// avr opcode SBC                   0000 10rd dddd rrrr, with d=dest register, r=source register
let SBC                           = (0x0800, MASK_DOUBLE_REG_OPERAND, "SBC")

// avr opcode SBCI                  0100 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
let SBCI                          = (0x4000, 0xF000, "SBCI")

// avr opcode SBIW                  1001 0111 KKdd KKKK, with d∈{24,26,28,30},0≤K≤63
let SBIW                          = (0x9700, 0xFF00, "SBIW")

// avr opcode SBRC                  1111 110r rrrr 0bbb, with r=a register and b=the bit to test
let SBRC                          = (0xFC00, 0xFE08, "SBRC")

// avr opcode SBRS                  1111 111r rrrr 0bbb, with r=a register and b=the bit to test
let SBRS                          = (0xFE00, 0xFE08, "SBRS")

// avr opcode SEI                   1001 0100 0111 1000
let SEI                           = (0x9478, 0xFFFF, "SEI")

// avr opcode SET                   1001 0100 0110 1000
let SET                           = (0x9468, 0xFFFF, "SET")

// avr opcode ST X, Rs              1001 001r rrrr 1100, with r=the register to store
let ST_X                          = (0x920C, MASK_SINGLE_REG_OPERAND, "ST_XINC")

// avr opcode ST X+, Rs             1001 001r rrrr 1101, with r=the register to store
let ST_XINC                       = (0x920D, MASK_SINGLE_REG_OPERAND, "ST_XINC")

// avr opcode ST -X, Rs             1001 001r rrrr 1110, with r=the register to store
let ST_DECX                       = (0x920E, MASK_SINGLE_REG_OPERAND, "ST_DECX")

// avr opcode ST Y, Rs              1000 001r rrrr 1000, with r=the register to store
let ST_Y                          = (0x8208, MASK_SINGLE_REG_OPERAND, "ST_YINC")

// avr opcode ST Y+, Rs             1001 001r rrrr 1001, with r=the register to store
let ST_YINC                       = (0x9209, MASK_SINGLE_REG_OPERAND, "ST_YINC")

// avr opcode ST -Y, Rs             1001 001r rrrr 1010, with r=the register to store
let ST_DECY                       = (0x920A, MASK_SINGLE_REG_OPERAND, "ST_DECY")

// avr opcode STD                   10q0 qq1r rrrr 1qqq, with r=source register, q=offset from Y or Z
let STD_Y                         = (0x8208, 0xD208, "STD_Y")

// avr opcode ST Y, Rs              1000 001r rrrr 0000, with r=the register to store
let ST_Z                          = (0x8200, MASK_SINGLE_REG_OPERAND, "ST_ZINC")

// avr opcode ST Z+, Rs             1001 001r rrrr 0001, with r=the register to store
let ST_ZINC                       = (0x9201, MASK_SINGLE_REG_OPERAND, "ST_ZINC")

// avr opcode ST -Z, Rs             1001 001r rrrr 1010, with r=the register to store
let ST_DECZ                       = (0x9202, MASK_SINGLE_REG_OPERAND, "ST_DECZ")

// avr opcode STD                   10q0 qq1r rrrr 0qqq, with r=source register, q=offset from Y or Z
let STD_Z                         = (0x8200, 0xD208, "STD_Z")

//                                  1001 001d dddd 0000
// avr opcode STS                   kkkk kkkk kkkk kkkk
let STS                           = (0x9200, MASK_SINGLE_REG_OPERAND, "STS")

// avr opcode SUB                   0001 10rd dddd rrrr, with d=dest register, r=source register
let SUB                           = (0x1800, MASK_DOUBLE_REG_OPERAND, "SUB")

// avr opcode SUBI                  0101 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
let SUBI                          = (0x5000, 0xF000, "SUBI")

let is (opcodeDefinition : int*int*string) inst =
    let (opcode, mask, name) = opcodeDefinition in
    (=) (inst &&& mask) opcode

let opcodeCategories = 
    [("01) LD/ST rel to X", [ LD_X; LD_XINC; LD_DECX; ST_X; ST_XINC; ST_DECX ]);
     ("02) LD/ST rel to Y", [ LD_Y; LD_YINC; LD_DECY; LDD_Y; ST_Y; ST_YINC; ST_DECY; STD_Y ]);
     ("03) LD/ST rel to Z", [ LD_Z; LD_ZINC; LD_DECZ; LDD_Z; ST_Z; ST_ZINC; ST_DECZ; STD_Z ]);
     ("04) Stack push/pop", [ POP; PUSH ]);
     ("05) Register moves", [ MOV; MOVW ]);         
     ("06) Constant load", [ LDI ]);
     ("07) Comp./branches", [ BRCC; BRCS; BREQ; BRGE; BRLT; BRNE; BRPL; CP; CPC; CPI; SBRC; SBRS ]);
     ("08) Math", [ ADC; ADD; ADIW; DEC; MUL; INC; SBC; SBCI; SBIW; SUB; SUBI ]);
     ("09) Bit shifts", [ ASR; ROR; LSR ]);
     ("10) Bit logic", [ AND; ANDI; EOR; OR; ORI ]);
     ("11) Subroutines", [ CALL; RCALL ]);
     ("12) Others", [ BREAK; NOP; RET; COM; IJMP; IN; JMP; LDS; RJMP; STS; SET; SEI; CLI; OUT; IN; BLD ])] in
let allOpcodes = opcodeCategories |> List.map snd |> List.concat
let getOpcodeForInstruction inst text =
    match allOpcodes |> List.tryFind (fun opcode -> is opcode inst) with
    | Some(opcode) -> opcode
    | None -> failwith (String.Format("No opcode found for 0x{0:X4} {1}", inst, text))

let opcodeCategory opcode =
    match opcodeCategories |> List.tryFind (fun (cat, opcodes) -> opcodes |> List.exists ((=) opcode)) with
    | Some(cat, _) -> cat
    | None -> "13) ????"
let opcodeName (opcode, mask, name) =
    name
let getAllOpcodeCategories =
    opcodeCategories |> List.map (fun (cat, opcodes) -> cat)


