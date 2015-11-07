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

let opcodeFromAvrInstr (x: AvrInstruction) =
    let opcode = x.Opcode.Trim() in
        Convert.ToInt32(opcode, 16)

let isPUSH x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFE0F) = OPCODE_PUSH || (x &&& 0xFE0F) = OPCODE_ST_XINC
let isPOP x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFE0F) = OPCODE_POP || (x &&& 0xFE0F) = OPCODE_LD_DECX
let isMOV x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFC00) = OPCODE_MOV
let isMOVW x = x |> opcodeFromAvrInstr |> fun x -> (x &&& 0xFF00) = OPCODE_MOVW
let isMOV_MOVW_PUSH_POP x = isMOV x || isMOVW x || isPUSH x || isPOP x




// Extracts the avr instructions from each jvm and returns a list of (avr, jvmindex) tuples
let jvmInstructionsToAvrAndJvmIndex (jvmInstructions : JavaInstruction seq) =
    jvmInstructions |> Seq.map (fun jvm -> jvm.UnoptimisedAvr.AvrInstructions |> Seq.map (fun avr -> (avr, jvm.Index)))
                    |> Seq.concat

let (|SeqEmpty|SeqCons|) (xs: 'a seq) =
  if Seq.isEmpty xs then SeqEmpty
  else SeqCons(Seq.head xs, Seq.skip 1 xs)

// Returns: a list of tuples (optimised avr instruction, jvm avr instruction, corresponding jvm index)
// The list ignores all PUSH/POP/MOVs. All remaining instructions should match exactly
// (todo: branches will break this)
let rec matchOptimisedAvrExceptPUSHPOPMOVMOVW (avrInstructions : AvrInstruction seq) (jvmInstructions : (AvrInstruction*int) seq) =
    match avrInstructions, jvmInstructions with
    // If the avr head is a MOV PUSH or POP, skip it
    | SeqCons(avrHead, avrTail), _ when isMOV_MOVW_PUSH_POP(avrHead)
        -> (Some avrHead, None, None) :: matchOptimisedAvrExceptPUSHPOPMOVMOVW avrTail jvmInstructions
    // If the jvm head is a MOV PUSH or POP, skip it
    | _, SeqCons((jvmHeadAvr, jvmHeadIdx), jvmTail) when isMOV_MOVW_PUSH_POP(jvmHeadAvr)
        -> (None, Some jvmHeadAvr, Some jvmHeadIdx) :: matchOptimisedAvrExceptPUSHPOPMOVMOVW avrInstructions jvmTail
    | SeqCons(avrHead, avrTail), SeqCons(jvmHead, jvmTail)
        -> []
    | _, _
        -> []

let rtcdata = RtcdataXml.Load("/Users/niels/git/rtc/src/build/avrora/rtcdata.py.xml")
let optimisedAvr = rtcdata.MethodImpls.First().AvrInstructions
let unoptimisedAvrWithJvmIndex = jvmInstructionsToAvrAndJvmIndex (rtcdata.MethodImpls.First().JavaInstructions)
let match1 = matchOptimisedAvrExceptPUSHPOPMOVMOVW optimisedAvr unoptimisedAvrWithJvmIndex

Console.WriteLine (match1.Count())





//let testPrintMethodImpl (methodImpl : MethodImpl) =
//    printfn "implId %d" methodImpl.MethodImplId
//    methodImpl.AvrInstructions.First().Opcode
