#r "binaries/FSharp.Data/FSharp.Data.dll"

open System
open System.IO
open System.Linq
open FSharp.Data

type RtcdataXml = XmlProvider<"rtcdata-example.xml", Global=true>
type Rtcdata = RtcdataXml.Methods
type JavaInstruction = RtcdataXml.JavaInstruction
type AvrInstruction = RtcdataXml.AvrInstruction
type MethodImpl = RtcdataXml.MethodImpl
type ProfilerdataXml = XmlProvider<"profilerdata-example.xml", Global=true>
type Profilerdata = ProfilerdataXml.ExecutionCountPerInstruction
type ProfiledInstruction = ProfilerdataXml.Instruction
type DarjeelingInfusionHeaderXml = XmlProvider<"infusionheader-example.dih", Global=true>
type Dih = DarjeelingInfusionHeaderXml.Dih
type ExecCounters = {
        executions : int;
        cycles : int; } with
     static member (+) (x, y) = {
        executions = x.executions + y.executions
        cycles = x.cycles + y.cycles }
     member x.average = if x.executions > 0
                        then float x.cycles / float x.executions
                        else 0.0

type ResultAvrInstruction = { unopt : AvrInstruction;
                               opt : AvrInstruction option;
                               counters : ExecCounters }
type Result = { jvm : JavaInstruction;
                avr : ResultAvrInstruction list;
                counters : ExecCounters }

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

// Input: the optimised avr code, and a list of tuples of unoptimised avr instructions and the jvm instruction that generated them
// Returns: a list of tuples (optimised avr instruction, unoptimised avr instruction, corresponding jvm index)
//          the optimised avr instruction may be None for instructions that were removed completely by the optimiser
let rec matchOptUnopt (optimisedAvr : AvrInstruction list) (unoptimisedAvr : (AvrInstruction*JavaInstruction) list) =
    match optimisedAvr, unoptimisedAvr with
    // Identical instructions: match and consume both
    | optimisedHead :: optimisedTail, (unoptimisedHead, jvmHead) :: unoptTail when optimisedHead.Text = unoptimisedHead.Text
        -> (Some optimisedHead, unoptimisedHead, jvmHead) :: matchOptUnopt optimisedTail unoptTail
    // Match a MOVW to two PUSH instructions (bit arbitrary whether to count the cycle for the PUSH or POP that was optimised)
    | optMOVW :: optTail, (unoptPUSH1, jvmHead1) :: (unoptPUSH2, jvmHead2) :: unoptTail when isMOVW(optMOVW) && isPUSH(unoptPUSH1) && isPUSH(unoptPUSH2)
        -> (Some optMOVW, unoptPUSH1, jvmHead1)
            :: (None, unoptPUSH2, jvmHead2)
            :: matchOptUnopt optTail unoptTail
    // If the unoptimised head is a MOV PUSH or POP, skip it
    | _, (unoptimisedHead, jvmHead) :: unoptTail when isMOV_MOVW_PUSH_POP(unoptimisedHead)
        -> (None, unoptimisedHead, jvmHead) :: matchOptUnopt optimisedAvr unoptTail
    // BREAK signals a branchtag that would have been replaced in the optimised code by a
    // branch to the real address, possibly followed by one or two NOPs
    | _, (unoptBranchtag, jvmBranchtag) :: (unoptBranchtag2, jvmBranchtag2) :: unoptTail when isBREAK(unoptBranchtag)
        -> match optimisedAvr with
            // Short conditional jump
            | optBR :: optNOP :: optNOP2 :: optTail when isBRANCH(optBR) && isNOP(optNOP) && isNOP(optNOP2)
                -> (Some optBR, unoptBranchtag, jvmBranchtag)
                    :: (Some optNOP, unoptBranchtag, jvmBranchtag)
                    :: (Some optNOP2, unoptBranchtag, jvmBranchtag)
                    :: matchOptUnopt optTail unoptTail
            // Mid range conditional jump
            | optBR :: optRJMP :: optNOP :: optTail when isBRANCH(optBR) && isRJMP(optRJMP) && isNOP(optNOP)
                -> (Some optBR, unoptBranchtag, jvmBranchtag)
                    :: (Some optRJMP, unoptBranchtag, jvmBranchtag)
                    :: (Some optNOP, unoptBranchtag, jvmBranchtag)
                    :: matchOptUnopt optTail unoptTail
            // Long conditional jump
            | optBR :: optJMP :: optTail when isBRANCH(optBR) && isJMP(optJMP)
                -> (Some optBR, unoptBranchtag, jvmBranchtag)
                    :: (Some optJMP, unoptBranchtag, jvmBranchtag)
                    :: matchOptUnopt optTail unoptTail
            // Uncondtional mid range jump
            | optRJMP :: optNOP :: optNOP2 :: optTail when isRJMP(optRJMP) && isNOP(optNOP) && isNOP(optNOP2)
                -> (Some optRJMP, unoptBranchtag, jvmBranchtag)
                    :: (Some optNOP, unoptBranchtag, jvmBranchtag)
                    :: (Some optNOP2, unoptBranchtag, jvmBranchtag)
                    :: matchOptUnopt optTail unoptTail
            // Uncondtional long jump
            | optJMP :: optNOP :: optTail when isJMP(optJMP) && isNOP(optNOP)
                -> (Some optJMP, unoptBranchtag, jvmBranchtag)
                    :: (Some optNOP, unoptBranchtag, jvmBranchtag)
                    :: matchOptUnopt optTail unoptTail
            | _ -> failwith "Incorrect branctag"
    | [], [] -> [] // All done.
    | _, _ -> failwith "Some instructions couldn't be matched"

// Input: the original Java instructions, trace data from avrora profiler, and the output from matchOptUnopt
// Returns: a list of Result records, showing the optimised code per original JVM instructions, and amount of cycles spent per optimised instruction
let rec addCycles (jvmInstructions : JavaInstruction list)
                  (profilerdata : ProfiledInstruction list)
                  (matchedResults : (AvrInstruction option*AvrInstruction*JavaInstruction) list) =
    jvmInstructions |> List.map
        (fun jvm ->
            let resultsForThisJvm = matchedResults |> List.filter (fun b -> let (_, _, jvm2) = b in jvm.Index = jvm2.Index) in
            let resultsWithCycles = resultsForThisJvm |> List.map (fun (opt, unopt, _) ->
                  let (executions, cycles) = match opt with
                                             | None -> (0, 0)
                                             | Some(optValue) ->
                                                 let address = Convert.ToInt32(optValue.Address.Trim(), 16) in
                                                 let profiledInstruction = profilerdata |> List.find (fun x -> Convert.ToInt32(x.Address.Trim(), 16) = address) in
                                                     (profiledInstruction.Executions, profiledInstruction.Cycles)
                  { unopt = unopt; opt = opt; counters = { executions = executions; cycles = cycles } }) in
            let avrCountersToJvmCounters a b =
                if a.executions > 0 // When adding avr counters to form the jvm counters, sum the cycles, and take the first execution count we encounter
                then { cycles = a.cycles+b.cycles; executions = a.executions }
                else { cycles = a.cycles+b.cycles; executions = b.executions }
            { jvm = jvm
              avr = resultsWithCycles
              counters = resultsWithCycles |> List.map (fun r -> r.counters) |> List.fold (avrCountersToJvmCounters) { executions=0; cycles=0 } })

let resultToString (results : Result list) =
    let totalCycles = results |> List.sumBy (fun r -> (r.counters.cycles))
    let countersToString (counters : ExecCounters) =
        String.Format("exe:{0,8} cyc:{1,10} {2:00.000}% avg: {3:00.00}",
                      counters.executions,
                      counters.cycles,
                      100.0 * float (counters.cycles) / float totalCycles,
                      counters.average)
    let jvmResultToString (result : Result) =
        String.Format("{0,-60}{1}\r\n",
                      result.jvm.Text,
                      countersToString result.counters)
    let avrResultsToString (avrResults : ResultAvrInstruction list) =
        let avrInstOption2Text = function
            | Some (x : AvrInstruction)
                -> String.Format("{0:10}: {1,-15}", x.Address, x.Text)
            | None -> "" in
        avrResults |> List.map (fun r -> String.Format("        {0,-15} -> {1,-36} {2,8} {3,14}\r\n",
                                                      r.unopt.Text,
                                                      avrInstOption2Text r.opt,
                                                      r.counters.executions,
                                                      r.counters.cycles))
                   |> List.fold (+) ""
    let r1 = "--- COMPLETE LISTING\r\n"
             + (results |> List.map (fun r -> (r |> jvmResultToString)
                                              + (r.avr |> avrResultsToString))
                        |> List.fold (+) "")
    let r2 = "--- ONLY OPTIMISED AVR\r\n"
             + (results |> List.map (fun r -> 
                                     (r |> jvmResultToString)
                                     + (r.avr |> List.filter (fun avr -> avr.opt.IsSome) |> avrResultsToString))
                        |> List.fold (+) "")
    let r3 = "--- ONLY JVM\r\n"
             + (results |> List.map (fun r -> 
                                     (r |> jvmResultToString))
                        |> List.fold (+) "")
    let jvmCategory opcode =
        let categories = 
                [("1) Ref stack ld/st", ["JVM_ALOAD_0"]);
                 ("2) Int stack ld/st", ["JVM_ILOAD"; "JVM_ILOAD_0"; "JVM_ILOAD_2"; "JVM_ISTORE"; "JVM_ISTORE_0"; "JVM_ISTORE_2"; "JVM_SLOAD"; "JVM_SSTORE"]);
                 ("3) Constant load", ["JVM_ICONST_0"; "JVM_ICONST_1"; "JVM_SIPUSH"]);
                 ("4) Array ld/st", ["JVM_SALOAD"; "JVM_SASTORE"]);
                 ("5) Branches", ["JVM_BRTARGET"; "JVM_GOTO"; "JVM_IF_ICMPGE"; "JVM_IF_SCMPLE"]);
                 ("6) Math", ["JVM_IINC"; "JVM_ISUB"])] in
        if categories.Any(fun (cat, opcodes) -> opcodes.Contains(opcode))
        then categories.First(fun (cat, opcodes) -> opcodes.Contains(opcode)) |> fst
        else "7) Others"
    let groupFold keyFunc valueFunc foldFunc foldInitAcc results =
        results |> List.toSeq
                |> Seq.groupBy keyFunc
                |> Seq.map (fun (key, groupedResults) -> (key, groupedResults |> Seq.map valueFunc |> Seq.fold foldFunc foldInitAcc))
                |> Seq.toList
    let summedPerJvmOpcode = results |> groupFold (fun r -> r.jvm.Text.Split().First()) (fun r -> r.counters) (+) { executions=0; cycles=0 }
                                     |> List.map (fun (opcode, counters) -> ((jvmCategory opcode), opcode, counters))
                                     |> List.sortBy (fun (category, opcode, _) -> category+opcode)
    let r4 = "--- SUMMED PER JVM OPCODE\r\n"
             + (summedPerJvmOpcode |> List.map (fun (category, opcode, counters) -> 
                                                    String.Format("{0,-20}{1,-20} total {2}\r\n",
                                                                  category,
                                                                  opcode,
                                                                  countersToString(counters)))
                                  |> List.fold (+) "")
    let summedPerJvmCategory = summedPerJvmOpcode |> groupFold (fun (cat,op,cnt) -> cat) (fun (cat,op,cnt) -> cnt) (+) { executions=0; cycles=0 }
                                                  |> List.sortBy (fun (category, _) -> category)
    let r5 = "--- SUMMED PER JVM CATEGORY\r\n"
             + (summedPerJvmCategory |> List.map (fun (category, counters) -> 
                                                      String.Format("{0,-40} total {1}\r\n",
                                                                    category,
                                                                    countersToString(counters)))
                                     |> List.fold (+) "")
    let r6 = "--- TOTAL CYCLES: " + totalCycles.ToString()
    r6 + "\r\n\r\n" + r5 + "\r\n\r\n" + r4 + "\r\n\r\n" + r3 + "\r\n\r\n" + r2 + "\r\n\r\n" + r1

// Find the methodImplId for a certain method in a Darjeeling infusion header
let findRtcbenchmarkMethodImplId (dih : Dih) methodName =
    let infusionName = dih.Infusion.Header.Name
    let methodDef = dih.Infusion.Methoddeflists |> Seq.find (fun def -> def.Name = methodName)
    let methodImpl = dih.Infusion.Methodimpllists |> Seq.find (fun impl -> impl.MethoddefEntityId = methodDef.EntityId && impl.MethoddefInfusion = infusionName)
    methodImpl.EntityId


// Process trace main function
let processTrace (dih : Dih) (rtcdata : Rtcdata) (profilerdata : Profilerdata) =
    let methodImplId = findRtcbenchmarkMethodImplId dih "rtcbenchmark_measure_java_performance"
    let methodImpl = rtcdata.MethodImpls |> Seq.find (fun impl -> impl.MethodImplId = methodImplId)

    let optimisedAvr = methodImpl.AvrInstructions |> Seq.toList
    let unoptimisedAvrWithJvmIndex =
        methodImpl.JavaInstructions |> Seq.map (fun jvm -> jvm.UnoptimisedAvr.AvrInstructions |> Seq.map (fun avr -> (avr, jvm)))
                                    |> Seq.concat
                                    |> Seq.toList

    let matchedResult = matchOptUnopt optimisedAvr unoptimisedAvrWithJvmIndex
    let matchedResultWithCycles = addCycles (methodImpl.JavaInstructions |> Seq.toList) (profilerdata.Instructions |> Seq.toList) matchedResult
    resultToString matchedResultWithCycles

let main(args : string[]) =
    if (args.Count() >= 5)
    then
        let dih = DarjeelingInfusionHeaderXml.Load(Array.get args 1)
        let rtcdata = RtcdataXml.Load(Array.get args 2)
        let profilerdata = ProfilerdataXml.Load(Array.get args 3)
        let results = processTrace dih rtcdata profilerdata
        File.WriteAllText ((Array.get args 4), results)
        Console.Error.WriteLine ("Wrote output to " + (Array.get args 4))
        1
    else
        let dih = DarjeelingInfusionHeaderXml.Load("/Users/niels/git/rtc/src/build/avrora/infusion-rtcbm_sort/rtcbm_sort.dih")
        let rtcdata = RtcdataXml.Load("/Users/niels/git/rtc/src/build/avrora/rtcdata.xml")
        let profilerdata = ProfilerdataXml.Load("/Users/niels/git/rtc/src/build/avrora/profilerdata.xml")
        let results = processTrace dih rtcdata profilerdata
        Console.WriteLine (results)
        0

main(fsi.CommandLineArgs)