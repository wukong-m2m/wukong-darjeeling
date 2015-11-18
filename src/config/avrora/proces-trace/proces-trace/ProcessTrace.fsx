#r "binaries/FSharp.Data/FSharp.Data.dll"
#r "binaries/fspickler.1.5.2/lib/net45/FsPickler.dll"

open System
open System.IO
open System.Linq
open System.Text.RegularExpressions
open System.Runtime.Serialization
open FSharp.Data
open Nessos.FsPickler

type RtcdataXml = XmlProvider<"rtcdata-example.xml", Global=true>
type Rtcdata = RtcdataXml.Methods
type MethodImpl = RtcdataXml.MethodImpl
type ProfilerdataXml = XmlProvider<"profilerdata-example.xml", Global=true>
type Profilerdata = ProfilerdataXml.ExecutionCountPerInstruction
type ProfiledInstruction = ProfilerdataXml.Instruction
type DarjeelingInfusionHeaderXml = XmlProvider<"infusionheader-example.dih", Global=true>
type Dih = DarjeelingInfusionHeaderXml.Dih


type AvrInstruction = {
    address : int;
    opcode : int;
    text : string; }
    with
    static member fromXml (xml : RtcdataXml.AvrInstruction) =
        {
            address = Convert.ToInt32(xml.Address.Trim(), 16);
            opcode = Convert.ToInt32((xml.Opcode.Trim()), 16);
            text = xml.Text;
        }

type JvmInstruction = {
    index : int
    text : string }
    with
    static member fromXml (xml : RtcdataXml.JavaInstruction) =
        {
            index = xml.Index;
            text = xml.Text;
        }

type ExecCounters = {
    executions : int;
    cycles : int; }
    with
    static member (+) (x, y) = { executions = x.executions + y.executions ; cycles = x.cycles + y.cycles }
    member x.average = if x.executions > 0
                       then float x.cycles / float x.executions
                       else 0.0

type ResultAvr = {
    unopt : AvrInstruction;
    opt : AvrInstruction option;
    counters : ExecCounters }

type ResultJava = {
    jvm : JvmInstruction;
    avr : ResultAvr list;
    counters : ExecCounters }

type Results = {
    jvmInstructions : ResultJava list;
    executedCyclesAOT : int;
    stopwatchCyclesC : int;
    stopwatchCyclesAOT : int;
    stopwatchCyclesJava : int;
    cyclesPush : ExecCounters;
    cyclesPop : ExecCounters;
    cyclesMovw : ExecCounters;
    cyclesPerJvmOpcode : list<string * ExecCounters>;
    cyclesPerJvmOpcodeCategory : list<string * ExecCounters>;
}

let jvmOpcodeCategories = 
        [("01) Ref stack ld/st", ["JVM_ALOAD"; "JVM_ALOAD_0"; "JVM_ALOAD_1"; "JVM_ALOAD_2"; "JVM_ALOAD_3"; "JVM_ASTORE"; "JVM_ASTORE_0"; "JVM_ASTORE_1"; "JVM_ASTORE_2"; "JVM_ASTORE_3"; "JVM_GETFIELD_A"; "JVM_PUTFIELD_A"; "JVM_GETSTATIC_A"; "JVM_PUTSTATIC_A"]);
         ("02) Int stack ld/st", ["JVM_SLOAD"; "JVM_SLOAD_0"; "JVM_SLOAD_1"; "JVM_SLOAD_2"; "JVM_SLOAD_3"; "JVM_ILOAD"; "JVM_ILOAD_0"; "JVM_ILOAD_1"; "JVM_ILOAD_2"; "JVM_ILOAD_3"; "JVM_SSTORE"; "JVM_SSTORE_0"; "JVM_SSTORE_1"; "JVM_SSTORE_2"; "JVM_SSTORE_3"; "JVM_ISTORE"; "JVM_ISTORE_0"; "JVM_ISTORE_1"; "JVM_ISTORE_2"; "JVM_ISTORE_3"; "JVM_IPOP"; "JVM_IPOP2"; "JVM_IDUP"; "JVM_IDUP2"; "JVM_IDUP_X"; "JVM_APOP"; "JVM_ADUP"; "JVM_GETFIELD_B"; "JVM_GETFIELD_C"; "JVM_GETFIELD_S"; "JVM_GETFIELD_I"; "JVM_PUTFIELD_B"; "JVM_PUTFIELD_C"; "JVM_PUTFIELD_S"; "JVM_PUTFIELD_I"; "JVM_GETSTATIC_B"; "JVM_GETSTATIC_C"; "JVM_GETSTATIC_S"; "JVM_GETSTATIC_I"; "JVM_PUTSTATIC_B"; "JVM_PUTSTATIC_C"; "JVM_PUTSTATIC_S"; "JVM_PUTSTATIC_I"]);
         ("03) Constant load", ["JVM_SCONST_M1"; "JVM_SCONST_0"; "JVM_SCONST_1"; "JVM_SCONST_2"; "JVM_SCONST_3"; "JVM_SCONST_4"; "JVM_SCONST_5"; "JVM_ICONST_M1"; "JVM_ICONST_0"; "JVM_ICONST_1"; "JVM_ICONST_2"; "JVM_ICONST_3"; "JVM_ICONST_4"; "JVM_ICONST_5"; "JVM_ACONST_NULL"; "JVM_BSPUSH"; "JVM_BIPUSH"; "JVM_SSPUSH"; "JVM_SIPUSH"; "JVM_IIPUSH"; "JVM_LDS"]);
         ("04) Array ld/st", ["JVM_BALOAD"; "JVM_CALOAD"; "JVM_SALOAD"; "JVM_IALOAD"; "JVM_AALOAD"; "JVM_BASTORE"; "JVM_CASTORE"; "JVM_SASTORE"; "JVM_IASTORE"; "JVM_AASTORE"]);
         ("05) Branches", ["JVM_SIFEQ"; "JVM_SIFNE"; "JVM_SIFLT"; "JVM_SIFGE"; "JVM_SIFGT"; "JVM_SIFLE"; "JVM_IIFEQ"; "JVM_IIFNE"; "JVM_IIFLT"; "JVM_IIFGE"; "JVM_IIFGT"; "JVM_IIFLE"; "JVM_IFNULL"; "JVM_IFNONNULL"; "JVM_IF_SCMPEQ"; "JVM_IF_SCMPNE"; "JVM_IF_SCMPLT"; "JVM_IF_SCMPGE"; "JVM_IF_SCMPGT"; "JVM_IF_SCMPLE"; "JVM_IF_ICMPEQ"; "JVM_IF_ICMPNE"; "JVM_IF_ICMPLT"; "JVM_IF_ICMPGE"; "JVM_IF_ICMPGT"; "JVM_IF_ICMPLE"; "JVM_IF_ACMPEQ"; "JVM_IF_ACMPNE"; "JVM_GOTO"; "JVM_TABLESWITCH"; "JVM_LOOKUPSWITCH"; "JVM_BRTARGET" ]);
         ("06) Math", ["JVM_SADD"; "JVM_SSUB"; "JVM_SMUL"; "JVM_SDIV"; "JVM_SREM"; "JVM_SNEG"; "JVM_IADD"; "JVM_ISUB"; "JVM_IMUL"; "JVM_IDIV"; "JVM_IREM"; "JVM_INEG"; "JVM_IINC"; "JVM_IINC_W"]);
         ("07) Bit shifts", ["JVM_SSHL"; "JVM_SSHR"; "JVM_SUSHR"; "JVM_ISHL"; "JVM_ISHR"; "JVM_IUSHR"]);
         ("08) Bit logic", ["JVM_SAND"; "JVM_SOR"; "JVM_SXOR"; "JVM_IAND"; "JVM_IOR"; "JVM_IXOR"]);
         ("09) Conversions", ["JVM_S2B"; "JVM_S2C"; "JVM_S2I"; "JVM_I2S"]);
         ("10) Others", ["JVM_NOP"; "JVM_SRETURN"; "JVM_IRETURN"; "JVM_ARETURN"; "JVM_RETURN"; "JVM_INVOKEVIRTUAL"; "JVM_INVOKESPECIAL"; "JVM_INVOKESTATIC"; "JVM_INVOKEINTERFACE"; "JVM_NEW"; "JVM_NEWARRAY"; "JVM_ANEWARRAY"; "JVM_ARRAYLENGTH"; "JVM_CHECKCAST"; "JVM_INSTANCEOF"])] in
let getCategoryForJvmOpcode opcode =
    if jvmOpcodeCategories.Any(fun (cat, opcodes) -> opcodes.Contains(opcode))
    then jvmOpcodeCategories.First(fun (cat, opcodes) -> opcodes.Contains(opcode)) |> fst
    else "11) ????"


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

let isPUSH x = (x.opcode &&& 0xFE0F) = OPCODE_PUSH || (x.opcode &&& 0xFE0F) = OPCODE_ST_XINC
let isPOP x = (x.opcode &&& 0xFE0F) = OPCODE_POP || (x.opcode &&& 0xFE0F) = OPCODE_LD_DECX
let isMOV x = (x.opcode &&& 0xFC00) = OPCODE_MOV
let isMOVW x = (x.opcode &&& 0xFF00) = OPCODE_MOVW
let isMOV_MOVW_PUSH_POP x = isMOV x || isMOVW x || isPUSH x || isPOP x
let isBREAK x = x.opcode = OPCODE_BREAK
let isNOP x = x.opcode = OPCODE_NOP
let isJMP x = (x.opcode &&& 0xFE0E0000) = OPCODE_MOVW
let isBREQ x = (x.opcode &&& 0xFC07) = OPCODE_BREQ
let isBRGE x = (x.opcode &&& 0xFC07) = OPCODE_BRGE
let isBRLT x = (x.opcode &&& 0xFC07) = OPCODE_BRLT
let isBRNE x = (x.opcode &&& 0xFC07) = OPCODE_BRNE
let isBRANCH x = isBREQ x || isBRGE x || isBRLT x || isBRNE x
let isRJMP x = (x.opcode &&& 0xF000) = OPCODE_RJMP

// Input: the optimised avr code, and a list of tuples of unoptimised avr instructions and the jvm instruction that generated them
// Returns: a list of tuples (optimised avr instruction, unoptimised avr instruction, corresponding jvm index)
//          the optimised avr instruction may be None for instructions that were removed completely by the optimiser
let rec matchOptUnopt (optimisedAvr : AvrInstruction list) (unoptimisedAvr : (AvrInstruction*JvmInstruction) list) =
    match optimisedAvr, unoptimisedAvr with
    // Identical instructions: match and consume both
    | optimisedHead :: optimisedTail, (unoptimisedHead, jvmHead) :: unoptTail when optimisedHead.text = unoptimisedHead.text
        -> (Some optimisedHead, unoptimisedHead, jvmHead) :: matchOptUnopt optimisedTail unoptTail
    // Match a MOV to a single PUSH instruction (bit arbitrary whether to count the cycle for the PUSH or POP that was optimised)
    | optMOV :: optTail, (unoptPUSH, jvmHead) :: unoptTail when isMOV(optMOV) && isPUSH(unoptPUSH)
        -> (Some optMOV, unoptPUSH, jvmHead)
            :: matchOptUnopt optTail unoptTail
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
let rec addCycles (jvmInstructions : JvmInstruction list) (profilerdata : ProfiledInstruction list) (matchedResults : (AvrInstruction option*AvrInstruction*JvmInstruction) list) =
    jvmInstructions |> List.map
        (fun jvm ->
            let resultsForThisJvm = matchedResults |> List.filter (fun b -> let (_, _, jvm2) = b in jvm.index = jvm2.index) in
            let resultsWithCycles = resultsForThisJvm |> List.map (fun (opt, unopt, _) ->
                  let (executions, cycles) = match opt with
                                             | None -> (0, 0)
                                             | Some(optValue) ->
                                                 let address = optValue.address in
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

let countPushPopMovw (results : ResultJava list) =
    let sumForOpcode predicate =
        results |> List.map (fun r -> r.avr)
                |> List.concat
                |> List.map (fun avr -> match avr.opt with
                                        | Some(avropt) when predicate avropt
                                          -> avr.counters
                                        | _
                                          -> { executions=0; cycles=0 })
                |> List.fold (+) { executions=0; cycles=0 }
    (sumForOpcode isPUSH, sumForOpcode isPOP, sumForOpcode isMOVW)

let getTimersFromStdout (stdoutlog : string list) =
    let pattern = "timer number (10\d): (\d+) cycles"
    stdoutlog |> List.map (fun line -> Regex.Match(line, pattern))
              |> List.filter (fun regexmatch -> regexmatch.Success)
              |> List.map (fun regexmatch -> (regexmatch.Groups.[1].Value, Int32.Parse(regexmatch.Groups.[2].Value)))
              |> List.map (fun (timer, cycles) ->
                                match timer with
                                | "101" -> ("C", cycles)
                                | "102" -> ("AOT", cycles)
                                | "103" -> ("Java", cycles)
                                | _ -> ("", cycles))
              |> List.sortBy (fun (timer, cycles) -> match timer with "C" -> 1 | "AOT" -> 2 | "Java" -> 3 | _ -> 4)

// Process trace main function
let processTrace (dih : Dih) (rtcdata : Rtcdata) (profilerdata : Profilerdata) (stdoutlog : string seq) =
    // Find the methodImplId for a certain method in a Darjeeling infusion header
    let findRtcbenchmarkMethodImplId (dih : Dih) methodName =
        let infusionName = dih.Infusion.Header.Name
        let methodDef = dih.Infusion.Methoddeflists |> Seq.find (fun def -> def.Name = methodName)
        let methodImpl = dih.Infusion.Methodimpllists |> Seq.find (fun impl -> impl.MethoddefEntityId = methodDef.EntityId && impl.MethoddefInfusion = infusionName)
        methodImpl.EntityId

    let methodImplId = findRtcbenchmarkMethodImplId dih "rtcbenchmark_measure_java_performance"
    let methodImpl = rtcdata.MethodImpls |> Seq.find (fun impl -> impl.MethodImplId = methodImplId)

    let optimisedAvr = methodImpl.AvrInstructions |> Seq.map AvrInstruction.fromXml |> Seq.toList
    let unoptimisedAvrWithJvmIndex =
        methodImpl.JavaInstructions |> Seq.map (fun jvm -> jvm.UnoptimisedAvr.AvrInstructions |> Seq.map (fun avr -> (AvrInstruction.fromXml avr, JvmInstruction.fromXml jvm)))
                                    |> Seq.concat
                                    |> Seq.toList

    let matchedResult = matchOptUnopt optimisedAvr unoptimisedAvrWithJvmIndex
    let matchedResultWithCycles = addCycles (methodImpl.JavaInstructions |> Seq.map JvmInstruction.fromXml |> Seq.toList) (profilerdata.Instructions |> Seq.toList) matchedResult
    let stopwatchTimers = getTimersFromStdout (stdoutlog |> Seq.toList)
    let (cyclesPush, cyclesPop , cyclesMovw) = (countPushPopMovw matchedResultWithCycles)

    let groupFold keyFunc valueFunc foldFunc foldInitAcc x =
        x |> List.toSeq
          |> Seq.groupBy keyFunc
          |> Seq.map (fun (key, groupedResults) -> (key, groupedResults |> Seq.map valueFunc |> Seq.fold foldFunc foldInitAcc))
          |> Seq.toList

    let cyclesPerJvmOpcode =
        matchedResultWithCycles
            |> List.filter (fun r -> r.jvm.text <> "Method preamble")
            |> groupFold (fun r -> r.jvm.text.Split().First()) (fun r -> r.counters) (+) { executions=0; cycles=0 }
            |> List.sortBy (fun (opcode, _) -> (getCategoryForJvmOpcode opcode)+opcode)

    let cyclesPerJvmOpcodeCategory =
        matchedResultWithCycles
            |> List.filter (fun r -> r.jvm.text <> "Method preamble")
            |> groupFold (fun r -> (getCategoryForJvmOpcode (r.jvm.text.Split().First()))) (fun r -> r.counters) (+) { executions=0; cycles=0 }
            |> List.sortBy (fun (category, _) -> category)


    {
        jvmInstructions = matchedResultWithCycles;
        executedCyclesAOT = matchedResultWithCycles |> List.sumBy (fun r -> (r.counters.cycles));
        stopwatchCyclesC = stopwatchTimers |> List.find (fun (t,c) -> t="C") |> snd;
        stopwatchCyclesAOT = stopwatchTimers |> List.find (fun (t,c) -> t="AOT") |> snd;
        stopwatchCyclesJava = stopwatchTimers |> List.find (fun (t,c) -> t="Java") |> snd;
        cyclesPush = cyclesPush;
        cyclesPop = cyclesPop;
        cyclesMovw = cyclesMovw;
        cyclesPerJvmOpcode = cyclesPerJvmOpcode;
        cyclesPerJvmOpcodeCategory = cyclesPerJvmOpcodeCategory;
    }

let resultsToString (results : Results) =
    let totalCycles = results.jvmInstructions |> List.sumBy (fun r -> (r.counters.cycles))
    let countersToString (counters : ExecCounters) =
        String.Format("exe:{0,8} cyc:{1,10} {2:00.000}% avg: {3:00.00}",
                      counters.executions,
                      counters.cycles,
                      100.0 * float (counters.cycles) / float totalCycles,
                      counters.average)
    let resultJavaToString (result : ResultJava) =
        String.Format("{0,-60}{1}\r\n",
                      result.jvm.text,
                      countersToString result.counters)
    let resultsAvrToString (avrResults : ResultAvr list) =
        let avrInstOption2Text = function
            | Some (x : AvrInstruction)
                -> String.Format("{0:10}: {1,-15}", x.address, x.text)
            | None -> "" in
        avrResults |> List.map (fun r -> String.Format("        {0,-15} -> {1,-36} {2,8} {3,14}\r\n",
                                                      r.unopt.text,
                                                      avrInstOption2Text r.opt,
                                                      r.counters.executions,
                                                      r.counters.cycles))
                   |> List.fold (+) ""

    let r1 = "--- COMPLETE LISTING\r\n"
             + (results.jvmInstructions
                    |> List.map (fun r -> (r |> resultJavaToString) + (r.avr |> resultsAvrToString))
                    |> List.fold (+) "")
    let r2 = "--- ONLY OPTIMISED AVR\r\n"
             + (results.jvmInstructions
                    |> List.map (fun r -> (r |> resultJavaToString) + (r.avr |> List.filter (fun avr -> avr.opt.IsSome) |> resultsAvrToString))
                    |> List.fold (+) "")
    let r3 = "--- ONLY JVM\r\n"
             + (results.jvmInstructions
                    |> List.map (fun r -> (r |> resultJavaToString))
                    |> List.fold (+) "")
    let r4 = "--- SUMMED PER JVM OPCODE\r\n"
             + (results.cyclesPerJvmOpcode
                    |> List.map (fun (opcode, counters)
                                     ->  String.Format("{0,-20}{1,-20} total {2}\r\n",
                                                       (getCategoryForJvmOpcode opcode),
                                                       opcode,
                                                       countersToString(counters)))
                    |> List.fold (+) "")
    let r5 = "--- SUMMED PER JVM CATEGORY\r\n"
             + (results.cyclesPerJvmOpcodeCategory
                    |> List.map (fun (category, counters)
                                     -> String.Format("{0,-40} total {1}\r\n",
                                                      category,
                                                      countersToString(counters)))
                    |> List.fold (+) "")
    let r6 = "--- TOTAL CYCLES SPENT IN COMPILED JVM CODE: " + totalCycles.ToString() + "\r\n    (doesn't include called functions)"

    let r7 = (String.Format ("--- TOTAL SPENT ON PUSH: {0}\r\n\
                              --- TOTAL SPENT ON POP:  {1}\r\n\
                              --- TOTAL SPENT ON MOVW: {2}\r\n\
                              --- COMBINED:            {3}",
                              (countersToString results.cyclesPush),
                              (countersToString results.cyclesPop),
                              (countersToString results.cyclesMovw),
                              (countersToString (results.cyclesPush + results.cyclesPop + results.cyclesMovw))))

    r7 + "\r\n\r\n" + r6 + "\r\n\r\n" + r5 + "\r\n\r\n" + r4 + "\r\n\r\n" + r3 + "\r\n\r\n" + r2 + "\r\n\r\n" + r1

let main(args : string[]) =
    if (args.Count() >= 5)
    then
        let dih = DarjeelingInfusionHeaderXml.Load(Array.get args 1)
        let rtcdata = RtcdataXml.Load(Array.get args 2)
        let profilerdata = ProfilerdataXml.Load(Array.get args 3)
        let stdoutlog = System.IO.File.ReadLines(Array.get args 4)
        let results = processTrace dih rtcdata profilerdata stdoutlog

        let txtFilename = (Array.get args 5) + ".txt"
        let xmlFilename = (Array.get args 5) + ".xml"
        File.WriteAllText (txtFilename, (resultsToString results))
        Console.Error.WriteLine ("Wrote output to " + txtFilename)

        let xmlSerializer = FsPickler.CreateXmlSerializer(indent = true)
        File.WriteAllText (xmlFilename, (xmlSerializer.PickleToString results))
        Console.Error.WriteLine ("Wrote output to " + xmlFilename)
        1
    else
        let dih = DarjeelingInfusionHeaderXml.Load("/Users/niels/src/rtc/src/build/avrora/infusion-bm_sortO/bm_sortO.dih")
        let rtcdata = RtcdataXml.Load("/Users/niels/src/rtc/src/build/avrora/rtcdata.xml")
        let profilerdata = ProfilerdataXml.Load("/Users/niels/src/rtc/src/build/avrora/profilerdata.xml")
        let stdoutlog = System.IO.File.ReadLines("/Users/niels/src/rtc/src/build/avrora/stdoutlog.txt")
        let results = processTrace dih rtcdata profilerdata stdoutlog
        Console.WriteLine (resultsToString results)
        0

//main([||])
main(fsi.CommandLineArgs)





















