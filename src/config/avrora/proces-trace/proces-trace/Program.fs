// Learn more about F# at http://fsharp.net
// See the 'F# Tutorial' project for more help.

open System
open System.Linq
open FSharp.Data

//[<EntryPoint>]
//let main argv = 
//    printfn "%A" argv
//    0 // return an integer exit code

type RtcdataXml = XmlProvider<"/Users/niels/git/rtc/src/build/avrora/rtcdata.py.xml", Global=true>
type JavaInstruction = RtcdataXml.JavaInstruction
type AvrInstruction = RtcdataXml.AvrInstruction
type MethodImpl = RtcdataXml.MethodImpl

[<Literal>]
let OPCODE_PUSH                    = 0x920F
[<Literal>]
let OPCODE_POP                     = 0x900F
[<Literal>]
let OPCODE_ST_XINC                 = 0x920D
[<Literal>]
let OPCODE_LD_DECX                 = 0x900E
[<Literal>]
let OPCODE_MOV                     = 0x2C00
[<Literal>]
let OPCODE_MOVW                    = 0x0100
[<Literal>]
let OPCODE_BREAK                   = 0x9598
[<Literal>]
let OPCODE_NOP                     = 0x0000
[<Literal>]
let OPCODE_JMP                     = 0x940C0000
[<Literal>]
let OPCODE_BREQ                    = 0xF001
[<Literal>]
let OPCODE_BRGE                    = 0xF404
[<Literal>]
let OPCODE_BRLT                    = 0xF004
[<Literal>]
let OPCODE_BRNE                    = 0xF401
[<Literal>]
let OPCODE_RJMP                    = 0xC000

let opcodeFromAvrInstr (x: AvrInstruction) =
    let opcode = x.Opcode.Trim() in
        Convert.ToInt32(opcode, 16)

let isPUSH x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFE0F) = OPCODE_PUSH || (x &&& 0xFE0F) = OPCODE_ST_XINC
let isPOP x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFE0F) = OPCODE_POP || (x &&& 0xFE0F) = OPCODE_LD_DECX
let isMOV x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFC00) = OPCODE_MOV
let isMOVW x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFF00) = OPCODE_MOVW
let isMOV_MOVW_PUSH_POP x = isMOV x || isMOVW x || isPUSH x || isPOP x
let isBREAK x = x |> opcodeFromAvrInstr |> (=) OPCODE_BREAK
let isNOP x = x |> opcodeFromAvrInstr |> (=) OPCODE_NOP
let isJMP x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFE0E0000) = OPCODE_MOVW
let isBREQ x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFC07) = OPCODE_BREQ
let isBRGE x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFC07) = OPCODE_BRGE
let isBRLT x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFC07) = OPCODE_BRLT
let isBRNE x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFC07) = OPCODE_BRNE
let isBRANCH x = isBREQ x || isBRGE x || isBRLT x || isBRNE x
let isRJMP x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xF000) = OPCODE_RJMP

// Extracts the avr instructions from each jvm and returns a list of (avr, jvmindex) tuples
let jvmInstructionsToAvrAndJvmIndex (jvmInstructions : JavaInstruction seq) =
    jvmInstructions |> Seq.map (fun jvm -> jvm.UnoptimisedAvr.AvrInstructions |> Seq.map (fun avr -> (avr, jvm.Index)))
                    |> Seq.concat
                    |> Seq.toList

// Returns: a list of tuples (optimised avr instruction, jvm avr instruction, corresponding jvm index)
// The list ignores all PUSH/POP/MOVs. All remaining instructions should match exactly
// (todo: branches will break this)
let rec match1 (optimisedAvr : AvrInstruction list) (unoptimisedAvr : (AvrInstruction*int) list) =
    match optimisedAvr, unoptimisedAvr with
    // Identical instructions: match and consume both
    | optimisedHead :: optimisedTail, (unoptimisedHead, jvmIdxHead) :: unoptTail when optimisedHead.Text = unoptimisedHead.Text
        -> (Some optimisedHead, Some unoptimisedHead, Some jvmIdxHead) :: match1 optimisedTail unoptTail
    // Match a MOVW to two PUSH instructions (bit arbitrary whether to count the cycle for the PUSH or POP that was optimised)
    | optMOVW :: optTail, (unoptPUSH1, jvmIdxHead1) :: (unoptPUSH2, jvmIdxHead2) :: unoptTail when isMOVW(optMOVW) && isPUSH(unoptPUSH1) && isPUSH(unoptPUSH2)
        -> (Some optMOVW, Some unoptPUSH1, Some jvmIdxHead1)
            :: (None, Some unoptPUSH2, Some jvmIdxHead2)
            :: match1 optTail unoptTail
    // If the unoptimised head is a MOV PUSH or POP, skip it
    | _, (unoptimisedHead, jvmIdxHead) :: unoptTail when isMOV_MOVW_PUSH_POP(unoptimisedHead)
        -> (None, Some unoptimisedHead, Some jvmIdxHead) :: match1 optimisedAvr unoptTail
    // If the optimised head is a MOV PUSH or POP, skip it
    | optimisedHead :: optimisedTail, _ when isMOV_MOVW_PUSH_POP(optimisedHead)
        -> (Some optimisedHead, None, None) :: match1 optimisedTail unoptimisedAvr
    // BREAK signals a branchtag that would have been replaced in the optimised code by a
    // branch to the real address, possibly followed by one or two NOPs
    | _, (unoptBranchtag, unoptBranchtagJvmIdx) :: (unoptBranchtag2, unoptBranchtagJvmIdx2) :: unoptTail when isBREAK(unoptBranchtag)
        -> match optimisedAvr with
            // Short conditional jump
            | optBR :: optNOP :: optNOP2 :: optTail when isBRANCH(optBR) && isNOP(optNOP) && isNOP(optNOP2)
                -> (Some optBR, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optNOP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optNOP2, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: match1 optTail unoptTail
            // Mid range conditional jump
            | optBR :: optRJMP :: optNOP :: optTail when isBRANCH(optBR) && isRJMP(optRJMP) && isNOP(optNOP)
                -> (Some optBR, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optRJMP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optNOP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: match1 optTail unoptTail
            // Long conditional jump
            | optBR :: optJMP :: optTail when isBRANCH(optBR) && isJMP(optJMP)
                -> (Some optBR, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optJMP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: match1 optTail unoptTail
            // Uncondtional mid range jump
            | optRJMP :: optNOP :: optNOP2 :: optTail when isRJMP(optRJMP) && isNOP(optNOP) && isNOP(optNOP2)
                -> (Some optRJMP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optNOP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optNOP2, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: match1 optTail unoptTail
            // Uncondtional long jump
            | optJMP :: optNOP :: optTail when isJMP(optJMP) && isNOP(optNOP)
                -> (Some optJMP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: (Some optNOP, Some unoptBranchtag, Some unoptBranchtagJvmIdx)
                    :: match1 optTail unoptTail
            | _ -> failwith "Incorrect branctag"
    | _, _
        -> []

let rtcdata = RtcdataXml.Load("/Users/niels/git/rtc/src/build/avrora/rtcdata.py.xml")
let optimisedAvr = rtcdata.MethodImpls.First().AvrInstructions |> Seq.toList
let unoptimisedAvrWithJvmIndex = jvmInstructionsToAvrAndJvmIndex (rtcdata.MethodImpls.First().JavaInstructions)
let matched = match1 optimisedAvr unoptimisedAvrWithJvmIndex

let rec printResult (avrInstructions : (AvrInstruction option*AvrInstruction option*int option) list) =
    let instOption2Text = function
        | Some (x : AvrInstruction) -> x.Text
        | None -> ""
    let instOption2Address = function
        | Some (x : AvrInstruction) -> x.Address
        | None -> ""
    match avrInstructions with
        | (optimisedAvr, unoptimisedAvr, jvmIndex) :: tail ->
                String.Format("{0,10} {1,20} {2,20} {3,4}\n",
                              instOption2Address optimisedAvr,
                              instOption2Text optimisedAvr,
                              instOption2Text unoptimisedAvr,
                              jvmIndex) + (printResult tail)
        | [] -> ""

Console.WriteLine (printResult matched)
Console.WriteLine (matched.Count())

//let testPrintMethodImpl (methodImpl : MethodImpl) =
//    printfn "implId %d" methodImpl.MethodImplId
//    methodImpl.AvrInstructions.First().Opcode
