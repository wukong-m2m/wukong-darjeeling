type AvrInstruction = {
    address : int;
    opcode : int;
    text : string; }

type JvmInstruction = {
    index : int
    text : string }

type ExecCounters = {
    executions : int;
    cycles : int; }
    with
    static member (+) (x, y) = { executions = x.executions + y.executions ; cycles = x.cycles + y.cycles }
    member this.average = if this.executions > 0
                          then float this.cycles / float this.executions
                          else 0.0
    static member empty = { executions = 0; cycles = 0 }

type ResultAvr = {
    unopt : AvrInstruction;
    opt : AvrInstruction option;
    counters : ExecCounters }

type StackDatatype = Ref | Byte | Char | Short | Int
let StackDatatypeToSize x =
    match x with // stack slot width is 16 bit, so everything takes at least two bytes.
    | Ref -> 2
    | Byte -> 2
    | Char -> 2
    | Short -> 2
    | Int -> 4
let StackDatatypeToString x =
    match x with
    | Ref -> "Ref"
    | Byte -> "Byte"
    | Char -> "Char"
    | Short -> "Short"
    | Int -> "Int"
let StackDatatypeFromString x =
    match x with
    | "Ref" -> Ref
    | "Byte" -> Byte
    | "Char" -> Char
    | "Short" -> Short
    | "Int" -> Int
    | _ -> failwith ("Unknown datatype " + x)

type StackElement = {
    origin : int
    datatype : StackDatatype
}

type DJDebugData = {
    byteOffset : int;
    instOffset : int;
    text : string;
    stackBefore : StackElement list;
    stackAfter : StackElement list; }
    with
    static member empty = { byteOffset = 0; instOffset = 0; text = ""; stackBefore = []; stackAfter = [] }
    member this.stackSizeBefore = this.stackBefore |> List.map (fun x -> x.datatype) |> List.sumBy StackDatatypeToSize
    member this.stackSizeAfter = this.stackAfter |> List.map (fun x -> x.datatype) |> List.sumBy StackDatatypeToSize

type ResultJava = {
    jvm : JvmInstruction;
    avr : ResultAvr list;
    counters : ExecCounters;
    djDebugData : DJDebugData; }

type Results = {
    benchmark : string;
    jvmInstructions : ResultJava list;
    nativeCInstructions : (AvrInstruction*ExecCounters) list
    stopwatchCyclesJava : int;
    stopwatchCyclesAOT : int;
    stopwatchCyclesC : int;
    codesizeJava : int;
    codesizeAOT : int;
    codesizeC : int;
    cyclesPush : ExecCounters;
    cyclesPop : ExecCounters;
    cyclesMovw : ExecCounters;
    cyclesPerJvmOpcode : (string * string * ExecCounters) list;
    cyclesPerAvrOpcodeAOTJava : (string * string * ExecCounters) list;
    cyclesPerAvrOpcodeNativeC : (string * string * ExecCounters) list;
    cyclesPerJvmOpcodeCategory : (string * ExecCounters) list;
    cyclesPerAvrOpcodeCategoryAOTJava : (string * ExecCounters) list;
    cyclesPerAvrOpcodeCategoryNativeC : (string * ExecCounters) list; }
    with
    member this.executedCyclesAOT = this.cyclesPerJvmOpcodeCategory |> List.sumBy (fun (cat, cnt) -> cnt.cycles);
    member this.executedCyclesC = this.cyclesPerAvrOpcodeCategoryNativeC |> List.sumBy (fun (cat, cnt) -> cnt.cycles);
    member this.maxJvmStackInBytes =
        this.jvmInstructions
        |> List.map (fun jvm -> jvm.djDebugData.stackSizeAfter)
        |> List.max;
    member this.avgJvmStackInBytes =
        this.jvmInstructions 
        |> List.fold (fun (accCnt, accSum) jvm -> (accCnt+jvm.counters.executions, accSum+(jvm.counters.executions*jvm.djDebugData.stackSizeAfter))) (0, 0)
        |> (fun (cnt, sum) -> float sum / float cnt)
    member this.avgJvmStackChangeInBytes =
        this.jvmInstructions 
        |> List.map (fun jvm -> (jvm.counters.executions, jvm.counters.executions * (abs (jvm.djDebugData.stackSizeBefore - jvm.djDebugData.stackSizeAfter))))
        |> List.fold (fun (accCnt, accSum) (jvmCnt, jvmSum) -> (accCnt+jvmCnt, accSum+jvmSum)) (0, 0)
        |> (fun (cnt, sum) -> float sum / float cnt)

