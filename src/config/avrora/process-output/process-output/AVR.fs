open System

//                                      1111 11rd dddd rrrr, with d=dest register, r=source register
let MASK_DOUBLE_REG_OPERAND             = 0xFC00
//                                      1111 111d dddd 1111
let MASK_SINGLE_REG_OPERAND             = 0xFE0F
//                                      1111 11kk kkkk k111, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let MASK_BRANCH                         = 0xFC07
//                                      1111 1111 1111 1111
let MASK_NO_OPERANDS                    = 0xFFFF

type AvrOpcode = {
    opcode : int
    mask : int
    text : string }
    with
    member this.is (inst : int) =
        (=) (inst &&& this.mask) this.opcode


// avr opcode ADC                   0001 11rd dddd rrrr, with d=dest register, r=source register
let ADC                           = { opcode=0x1C00; mask=MASK_DOUBLE_REG_OPERAND; text="ADC" }

// avr opcode ADD                   0000 11rd dddd rrrr, with d=dest register, r=source register
let ADD                           = { opcode=0x0C00; mask=MASK_DOUBLE_REG_OPERAND; text="ADD" }

// avr opcode ADIW                  1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
let ADIW                          = { opcode=0x9600; mask=0xFF00; text="ADIW" }

// avr opcode AND                   0010 00rd dddd rrrr, with d=dest register, r=source register
let AND                           = { opcode=0x2000; mask=MASK_DOUBLE_REG_OPERAND; text="AND" }

// avr opcode ANDI                  0111 KKKK dddd KKKK, with d=dest register-16, K=8 bit constant
let ANDI                          = { opcode=0x7000; mask=0xF000; text="ANDI" }

// avr opcode ASR                   1001 010d dddd 0101
let ASR                           = { opcode=0x9405; mask=MASK_SINGLE_REG_OPERAND; text="ASR" }

// avr opcode BLD                   1111 100d dddd 0bbb, with d=dest register, 0<=b<=7
let BLD                           = { opcode=0xF800; mask=0xFE08; text="BLD" }

// avr opcode BREAK                 1001 0101 1001 1000
let BREAK                         = { opcode=0x9598; mask=MASK_NO_OPERANDS; text="BREAK" }

// avr opcode BRCC                  1111 01kk kkkk k000
let BRCC                          = { opcode=0xF400; mask=MASK_BRANCH; text="BRCC" }

// avr opcode BRCS                  1111 00kk kkkk k000
let BRCS                          = { opcode=0xF000; mask=MASK_BRANCH; text="BRCS" }

// avr opcode BREQ                  1111 00kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BREQ                          = { opcode=0xF001; mask=MASK_BRANCH; text="BREQ" }

// avr opcode BRGE                  1111 01kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRGE                          = { opcode=0xF404; mask=MASK_BRANCH; text="BRGE" }

// avr opcode BRLO                  1111 00kk kkkk k000, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRLO                          = { opcode=0xF000; mask=MASK_BRANCH; text="BRLO" }

// avr opcode BRLT                  1111 00kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRLT                          = { opcode=0xF004; mask=MASK_BRANCH; text="BRLT" }

// avr opcode BRNE                  1111 01kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRNE                          = { opcode=0xF401; mask=MASK_BRANCH; text="BRNE" }

// avr opcode BRPL                  1111 01kk kkkk k010
let BRPL                          = { opcode=0xF402; mask=MASK_BRANCH; text="BRPL" }

// avr opcode BRSH                  1111 01kk kkkk k000, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
let BRSH                          = { opcode=0xF400; mask=MASK_BRANCH; text="BRSH" }

//                                  1001 010k kkkk 111k
// avr opcode CALL                  kkkk kkkk kkkk kkkk
let CALL                          = { opcode=0x940E; mask=0xFE0E; text="CALL" }

// avr opcode COM                   1001 010d dddd 0000, with d=dest register
let COM                           = { opcode=0x9400; mask=MASK_SINGLE_REG_OPERAND; text="COM" }

// avr opcode CLI                   1001 0100 1111 1000
let CLI                           = { opcode=0x94F8; mask=MASK_NO_OPERANDS; text="CLI" }

// avr opcode CLT                   1001 0100 1110 1000
let CLT                           = { opcode=0x94E8; mask=MASK_NO_OPERANDS; text="CLT" }

// avr opcode CP                    0001 01rd dddd rrrr, with r,d=the registers to compare
let CP                            = { opcode=0x1400; mask=MASK_DOUBLE_REG_OPERAND; text="CP" }

// avr opcode CPC                   0000 01rd dddd rrrr, with r,d=the registers to compare
let CPC                           = { opcode=0x0400; mask=MASK_DOUBLE_REG_OPERAND; text="CPC" }

// avr opcode CPI                   0011 KKKK dddd KKKK, with 16 ≤ d ≤ 31, 0≤ K ≤ 255
let CPI                           = { opcode=0x3000; mask=0xF000; text="CPI" }

// avr opcode CPSE                  0001 00rd dddd rrrr, with r,d=the registers to compare
let CPSE                          = { opcode=0x1000; mask=MASK_DOUBLE_REG_OPERAND; text="CPSE" }

// avr opcode DEC                   1001 010d dddd 1010
let DEC                           = { opcode=0x940A; mask=MASK_SINGLE_REG_OPERAND; text="DEC" }

// avr opcode EOR                   0010 01rd dddd rrrr, with d=dest register, r=source register
let EOR                           = { opcode=0x2400; mask=MASK_DOUBLE_REG_OPERAND; text="EOR" }

// avr opcode IJMP                  1001 0100 0000 1001
let IJMP                          = { opcode=0x9409; mask=MASK_NO_OPERANDS; text="IJMP" }

// avr opcode IN                    1011 0AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
let IN                            = { opcode=0xB000; mask=0xF800; text="IN" }

// avr opcode INC                   1001 010d dddd 0011, with d=dest register
let INC                           = { opcode=0x9403; mask=MASK_SINGLE_REG_OPERAND; text="INC" }

//                                  1001 010k kkkk 110k
// avr opcode JMP                   kkkk kkkk kkkk kkkk, with k the address in WORDS, not bytes. PC <- k
let JMP                           = { opcode=0x940C; mask=0xFE0E; text="JMP" }

// avr opcode LD Rd, X              1001 000d dddd 1100, with d=dest register
let LD_X                          = { opcode=0x900C; mask=MASK_SINGLE_REG_OPERAND; text="LD_X" }

// avr opcode LD Rd, X+             1001 000d dddd 1101
let LD_XINC                       = { opcode=0x900D; mask=MASK_SINGLE_REG_OPERAND; text="LD_XINC" }

// avr opcode LD Rd, -X             1001 000d dddd 1110, with d=dest register
let LD_DECX                       = { opcode=0x900E; mask=MASK_SINGLE_REG_OPERAND; text="LD_DECX" }

// avr opcode LD Rd, Y              1000 000d dddd 1000, with d=dest register
let LD_Y                          = { opcode=0x8008; mask=MASK_SINGLE_REG_OPERAND; text="LD_Y" }

// avr opcode LD Rd, Y+             1001 000d dddd 1001
let LD_YINC                       = { opcode=0x9009; mask=MASK_SINGLE_REG_OPERAND; text="LD_YINC" }

// avr opcode LD Rd, -Y             1001 000d dddd 1010, with d=dest register
let LD_DECY                       = { opcode=0x900A; mask=MASK_SINGLE_REG_OPERAND; text="LD_DECY" }

// avr opcode LDD_Y                 10q0 qq0d dddd 1qqq, with d=dest register, q=offset from Y
let LDD_Y                         = { opcode=0x8008; mask=0xD208; text="LDD_Y" }

// avr opcode LD Rd, Z              1000 000d dddd 0000, with d=dest register
let LD_Z                          = { opcode=0x8000; mask=MASK_SINGLE_REG_OPERAND; text="LD_Z" }

// avr opcode LD Rd, Z+             1001 000d dddd 0001
let LD_ZINC                       = { opcode=0x9001; mask=MASK_SINGLE_REG_OPERAND; text="LD_ZINC" }

// avr opcode LD Rd, -Z             1001 000d dddd 0010, with d=dest register
let LD_DECZ                       = { opcode=0x9002; mask=MASK_SINGLE_REG_OPERAND; text="LD_DECZ" }

// avr opcode LDD_Z                 10q0 qq0d dddd 0qqq, with d=dest register, q=offset from Z
let LDD_Z                         = { opcode=0x8000; mask=0xD208; text="LDD_Z" }

// avr opcode LDI                   1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
let LDI                           = { opcode=0xE000; mask=0xF000; text="LDI" }

//                                  1001 000d dddd 0000
// avr opcode LDS                   kkkk kkkk kkkk kkkk
let LDS                           = { opcode=0x9000; mask=MASK_SINGLE_REG_OPERAND; text="LDS" }

// avr opcode LPM                   1001 0101 1100 1000, r0 <- Z
let LPM                           = { opcode=0x95C8; mask=MASK_NO_OPERANDS; text="LPM" }

// avr opcode LPM_Z                 1001 000d dddd 0100, with d=dest register
let LPM_Z                         = { opcode=0x9004; mask=MASK_SINGLE_REG_OPERAND; text="LPM_Z" }

// avr opcode LPM_ZINC              1001 000d dddd 0101, with d=dest register
let LPM_ZINC                      = { opcode=0x9005; mask=MASK_SINGLE_REG_OPERAND; text="LPM_ZINC" }

// avr opcode LSR                   1001 010d dddd 0110
let LSR                           = { opcode=0x9406; mask=MASK_SINGLE_REG_OPERAND; text="LSR" }

// avr opcode MOV                   0010 11rd dddd rrrr, with d=dest register, r=source register
let MOV                           = { opcode=0x2C00; mask=MASK_DOUBLE_REG_OPERAND; text="MOV" }

// avr opcode MOVW                  0000 0001 dddd rrrr, with d=dest register/2, r=source register/2
let MOVW                          = { opcode=0x0100; mask=0xFF00; text="MOVW" }

// avr opcode MUL                   1001 11rd dddd rrrr, with d=dest register, r=source register
let MUL                           = { opcode=0x9C00; mask=MASK_DOUBLE_REG_OPERAND; text="MUL" }

// avr opcode MULS                  0000 0010 dddd rrrr, with d=source1 register-16, r=source2 register-16 (result in r1:r0)
let MULS                          = { opcode=0x0200; mask=0xFF00; text="MULS" }

// avr opcode MULSU                 0000 0011 0ddd 0rrr, with d=source1 register-16 (r16-r23), r=source2 register-16 (r16-r23) (result in r1:r0)
let MULSU                         = { opcode=0x0300; mask=0xFF88; text="MULSU" }

// avr opcode NEG                   1001 010d dddd 0001, with d=dest register
let NEG                           = { opcode=0x9401; mask=MASK_SINGLE_REG_OPERAND; text="NEG" }

// avr opcode NOP                   0000 0000 0000 0000
let NOP                           = { opcode=0x0000; mask=MASK_NO_OPERANDS; text="NOP" }

// avr opcode OR                    0010 10rd dddd rrrr, with d=dest register, r=source register
let OR                            = { opcode=0x2800; mask=MASK_DOUBLE_REG_OPERAND; text="OR" }

// avr opcode ORI                   0110 KKKK dddd KKKK, with d=dest register-16, K=8 bit constant
let ORI                           = { opcode=0x6000; mask=0xF000; text="ORI" }

// avr opcode OUT                   1011 1AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
let OUT                           = { opcode=0xB800; mask=0xF800; text="OUT" }

// avr opcode PUSH                  1001 001d dddd 1111, with d=source register
let PUSH                          = { opcode=0x920F; mask=MASK_SINGLE_REG_OPERAND; text="PUSH" }

// avr opcode POP                   1001 000d dddd 1111
let POP                           = { opcode=0x900F; mask=MASK_SINGLE_REG_OPERAND; text="POP" }

// avr opcode RCALL                 1101 kkkk kkkk kkkk, with k relative in words, not bytes. PC <- PC + k + 1
let RCALL                         = { opcode=0xD000; mask=0xF000; text="RCALL" }

// avr opcode RET                   1001 0101 0000 1000
let RET                           = { opcode=0x9508; mask=MASK_NO_OPERANDS; text="RET" }

// avr opcode RJMP                  1100 kkkk kkkk kkkk, with k the signed offset to jump to, in WORDS, not bytes. PC <- PC + k + 1
let RJMP                          = { opcode=0xC000; mask=0xF000; text="RJMP" }

// avr opcode ROR                   1001 010d dddd 0111
let ROR                           = { opcode=0x9407; mask=MASK_SINGLE_REG_OPERAND; text="ROR" }

// avr opcode SBC                   0000 10rd dddd rrrr, with d=dest register, r=source register
let SBC                           = { opcode=0x0800; mask=MASK_DOUBLE_REG_OPERAND; text="SBC" }

// avr opcode SBCI                  0100 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
let SBCI                          = { opcode=0x4000; mask=0xF000; text="SBCI" }

// avr opcode SBIW                  1001 0111 KKdd KKKK, with d∈{24,26,28,30},0≤K≤63
let SBIW                          = { opcode=0x9700; mask=0xFF00; text="SBIW" }

// avr opcode SBRC                  1111 110r rrrr 0bbb, with r=a register and b=the bit to test
let SBRC                          = { opcode=0xFC00; mask=0xFE08; text="SBRC" }

// avr opcode SBRS                  1111 111r rrrr 0bbb, with r=a register and b=the bit to test
let SBRS                          = { opcode=0xFE00; mask=0xFE08; text="SBRS" }

// avr opcode SEI                   1001 0100 0111 1000
let SEI                           = { opcode=0x9478; mask=MASK_NO_OPERANDS; text="SEI" }

// avr opcode SET                   1001 0100 0110 1000
let SET                           = { opcode=0x9468; mask=MASK_NO_OPERANDS; text="SET" }

// avr opcode ST X, Rs              1001 001r rrrr 1100, with r=the register to store
let ST_X                          = { opcode=0x920C; mask=MASK_SINGLE_REG_OPERAND; text="ST_XINC" }

// avr opcode ST X+, Rs             1001 001r rrrr 1101, with r=the register to store
let ST_XINC                       = { opcode=0x920D; mask=MASK_SINGLE_REG_OPERAND; text="ST_XINC" }

// avr opcode ST -X, Rs             1001 001r rrrr 1110, with r=the register to store
let ST_DECX                       = { opcode=0x920E; mask=MASK_SINGLE_REG_OPERAND; text="ST_DECX" }

// avr opcode ST Y, Rs              1000 001r rrrr 1000, with r=the register to store
let ST_Y                          = { opcode=0x8208; mask=MASK_SINGLE_REG_OPERAND; text="ST_YINC" }

// avr opcode ST Y+, Rs             1001 001r rrrr 1001, with r=the register to store
let ST_YINC                       = { opcode=0x9209; mask=MASK_SINGLE_REG_OPERAND; text="ST_YINC" }

// avr opcode ST -Y, Rs             1001 001r rrrr 1010, with r=the register to store
let ST_DECY                       = { opcode=0x920A; mask=MASK_SINGLE_REG_OPERAND; text="ST_DECY" }

// avr opcode STD                   10q0 qq1r rrrr 1qqq, with r=source register, q=offset from Y or Z
let STD_Y                         = { opcode=0x8208; mask=0xD208; text="STD_Y" }

// avr opcode ST Y, Rs              1000 001r rrrr 0000, with r=the register to store
let ST_Z                          = { opcode=0x8200; mask=MASK_SINGLE_REG_OPERAND; text="ST_ZINC" }

// avr opcode ST Z+, Rs             1001 001r rrrr 0001, with r=the register to store
let ST_ZINC                       = { opcode=0x9201; mask=MASK_SINGLE_REG_OPERAND; text="ST_ZINC" }

// avr opcode ST -Z, Rs             1001 001r rrrr 1010, with r=the register to store
let ST_DECZ                       = { opcode=0x9202; mask=MASK_SINGLE_REG_OPERAND; text="ST_DECZ" }

// avr opcode STD                   10q0 qq1r rrrr 0qqq, with r=source register, q=offset from Y or Z
let STD_Z                         = { opcode=0x8200; mask=0xD208; text="STD_Z" }

//                                  1001 001d dddd 0000
// avr opcode STS                   kkkk kkkk kkkk kkkk
let STS                           = { opcode=0x9200; mask=MASK_SINGLE_REG_OPERAND; text="STS" }

// avr opcode SUB                   0001 10rd dddd rrrr, with d=dest register, r=source register
let SUB                           = { opcode=0x1800; mask=MASK_DOUBLE_REG_OPERAND; text="SUB" }

// avr opcode SUBI                  0101 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
let SUBI                          = { opcode=0x5000; mask=0xF000; text="SUBI" }

// avr opcode SWAP                  1001 010d dddd 0010
let SWAP                          = { opcode=0x9402; mask=MASK_SINGLE_REG_OPERAND; text="SWAP" }


let opcodeCategories = 
    [("01) LD/ST rel to X", [ LD_X; LD_XINC; LD_DECX; ST_X; ST_XINC; ST_DECX ]);
     ("02) LD/ST rel to Y", [ LD_Y; LD_YINC; LD_DECY; LDD_Y; ST_Y; ST_YINC; ST_DECY; STD_Y ]);
     ("03) LD/ST rel to Z", [ LD_Z; LD_ZINC; LD_DECZ; LDD_Z; ST_Z; ST_ZINC; ST_DECZ; STD_Z ]);
     ("04) Stack push/pop", [ POP; PUSH ]);
     ("05) Register moves", [ MOV; MOVW ]);         
     ("06) Constant load", [ LDI ]);
     ("07) Comp./branches", [ BRCC; BRCS; BREQ; BRGE; BRLO; BRLT; BRNE; BRPL; BRSH; CP; CPC; CPI; CPSE; SBRC; SBRS ]);
     ("08) Math", [ ADC; ADD; ADIW; DEC; MUL; MULS; MULSU; NEG; INC; SBC; SBCI; SBIW; SUB; SUBI ]);
     ("09) Bit shifts", [ ASR; ROR; LSR ]);
     ("10) Bit logic", [ AND; ANDI; EOR; OR; ORI ]);
     ("11) Subroutines", [ CALL; RCALL ]);
     ("12) Others", [ BREAK; NOP; RET; COM; IJMP; IN; JMP; LDS; RJMP; STS; SET; SEI; CLI; CLT; OUT; IN; BLD; LPM; LPM_Z; LPM_ZINC; SWAP ])] in
let allOpcodes = opcodeCategories |> List.map snd |> List.concat

let getTargetIfCALL inst =
    match (CALL.is inst) with
    | false -> 0
    | true ->
        //                                   1001 010k kkkk 111k
        // avr opcode CALL                   kkkk kkkk kkkk kkkk
        // words in reversed order, ex: 0x865D940E is a CALL to 0x865D
                                          // llll llll llll llll 0000 000h hhhh 000h
        2*((((inst >>> 16) &&& 0xFFFF)                 // llll llll llll llll 0000 0000 0000 0000
           + ((inst &&& 0x0001) <<< 16)   // 0000 0000 0000 0000 0000 0000 0000 000h
           + ((inst &&& 0x01F0) <<< 13))) // 0000 0000 0000 0000 0000 000h hhhh 0000

let getOpcodeForInstruction inst text (addressesOfMathFunctions : (string * int) list) =
    match getTargetIfCALL inst with
    | 0 -> match allOpcodes |> List.tryFind (fun opcode -> opcode.is inst) with // Not a call
           | Some(opcode) -> opcode
           | None -> failwith (String.Format("No opcode found for 0x{0:X4} {1}", inst, text))
    | targetAddress ->
        match addressesOfMathFunctions |> List.tryFind (fun (name, address) -> address = targetAddress) with
        | None -> CALL // It's a call, but not to a math addressesOfMathFunctions
        | Some(name, address) -> { CALL with text = "CALL " + name }

let opcodeCategory opcode =
    if opcode.opcode = CALL.opcode
    then match opcode.text with
         | "CALL" -> "11) Subroutines" // Just CALL : count as normal subroutine
         | _ -> "08) Math"             // CALLs to math functions will have the name of the called function appended to the text
    else match opcodeCategories |> List.tryFind (fun (cat, opcodes) -> opcodes |> List.exists ((=) opcode)) with
         | Some(cat, _) -> cat
         | None -> "13) ????"

let opcodeName (opcode : AvrOpcode) =
    opcode.text

let instructionSize inst =
    let opcode = getOpcodeForInstruction inst (inst.ToString()) []
    if (opcode = CALL || opcode = JMP || opcode = LDS || opcode = STS) then 4 else 2

let getAllOpcodeCategories =
    opcodeCategories |> List.map (fun (cat, opcodes) -> cat)


