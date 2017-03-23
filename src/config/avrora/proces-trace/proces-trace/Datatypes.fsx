#r "binaries/FSharp.Data/FSharp.Data.dll"
#load "Helpers.fsx"
#load "AVR.fsx"
#load "JVM.fsx"

open Helpers
open FSharp.Data
open System.Web

type RtcdataXml = XmlProvider<"rtcdata-example.xml", Global=true>
type Rtcdata = RtcdataXml.Methods
type MethodImpl = RtcdataXml.MethodImpl
type ProfilerdataXml = XmlProvider<"profilerdata-example.xml", Global=true>
type Profilerdata = ProfilerdataXml.ExecutionCountPerInstruction
type ProfiledInstruction = ProfilerdataXml.Instruction

let getMethodNameFromFullName (fullname : string) =
    let indexOfLastDot = fullname.LastIndexOf(".");
    let indexOfSecondLastDot = match indexOfLastDot with
                               | -1 -> -1
                               | 0 -> -1
                               | _ -> fullname.LastIndexOf(".", indexOfLastDot-1)
    let methodName = match indexOfSecondLastDot with
                     | -1 -> fullname
                     | _ -> fullname.Substring(indexOfSecondLastDot+1)
    HttpUtility.UrlDecode(methodName)

let getMethodNameFromImpl (impl : MethodImpl) =
    getMethodNameFromFullName impl.Method

type AvrInstruction = {
    address : int
    opcode : int
    text : string }

type JvmInstruction = {
    index : int
    text : string }

type ExecCounters = {
    executions : int
    cycles : int
    count : int
    size : int }
    with
    static member (+) (x, y) = { executions = x.executions + y.executions ; cycles = x.cycles + y.cycles ; count = x.count + y.count ; size = x.size + y.size }
    static member (-) (x, y) = { executions = x.executions - y.executions ; cycles = x.cycles - y.cycles ; count = x.count - y.count ; size = x.size - y.size }
    member this.average = if this.executions > 0
                          then float this.cycles / float this.executions
                          else 0.0
    static member empty = { executions = 0; cycles = 0; count = 0; size = 0 }

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

type ProcessedJvmInstruction = {
    jvm : JvmInstruction;
    avr : ResultAvr list;
    counters : ExecCounters;
    djDebugData : DJDebugData; }

let sumCyclesCategoryCounters = 
  (fun (cyclesPerAvrOpcodeCategory) (catTest : string -> bool) -> cyclesPerAvrOpcodeCategory |> List.filter (fun (cat,cnt) -> (catTest cat))
                                                                                             |> List.map snd
                                                                                             |> List.fold (+) ExecCounters.empty)

let groupOpcodesInCategoriesCycles (allCategories : string list) (results : (string * string * ExecCounters) list) =
    let categoriesPresent =
        results
            |> List.map (fun (cat, opcode, cnt) -> (cat, cnt))
            |> groupFold fst snd (+) ExecCounters.empty
    allCategories
        |> List.sort
        |> List.map (fun cat -> match categoriesPresent |> List.tryFind (fun (cat2, _) -> cat = cat2) with
                                | Some (cat, cnt) -> (cat, cnt)
                                | None -> (cat, ExecCounters.empty))

let groupOpcodesInCategoriesBytes (allCategories : string list) (results : (string * string * int) list) =
    let categoriesPresent =
        results
            |> List.map (fun (cat, opcode, cnt) -> (cat, cnt))
            |> groupFold fst snd (+) 0
    allCategories
        |> List.sort
        |> List.map (fun cat -> match categoriesPresent |> List.tryFind (fun (cat2, _) -> cat = cat2) with
                                | Some (cat, cnt) -> (cat, cnt)
                                | None -> (cat, 0))

type SimulationResults = {
    benchmark : string;
    jvmInstructions : ProcessedJvmInstruction list;
    nativeCInstructions : (AvrInstruction*ExecCounters) list
    passedTestJava : bool;
    passedTestAOT : bool;

    countersPerJvmOpcodeAOTJava : (string * string * ExecCounters) list;
    countersPerAvrOpcodeAOTJava : (string * string * ExecCounters) list;
    countersPerAvrOpcodeNativeC : (string * string * ExecCounters) list;

    codesizeJava : int;
    codesizeJavaBranchCount : int;
    codesizeJavaBranchTargetCount : int;
    codesizeJavaMarkloopCount : int;
    codesizeJavaMarkloopTotalSize : int;
    codesizeJavaWithoutBranchOverhead : int;
    codesizeJavaWithoutBranchMarkloopOverhead : int;
    codesizeAOT : int;
    codesizeC : int;

    cyclesStopwatchJava : int;
    cyclesStopwatchAOT : int;
    cyclesStopwatchC : int; }
    with
    member this.countersPerJvmOpcodeCategoryAOTJava = groupOpcodesInCategoriesCycles JVM.getAllJvmOpcodeCategories this.countersPerJvmOpcodeAOTJava
    member this.countersPerAvrOpcodeCategoryNativeC = groupOpcodesInCategoriesCycles AVR.getAllOpcodeCategories    this.countersPerAvrOpcodeNativeC
    member this.countersPerAvrOpcodeCategoryAOTJava = groupOpcodesInCategoriesCycles AVR.getAllOpcodeCategories    this.countersPerAvrOpcodeAOTJava

    member this.countersCLoadStore        = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> cat.Contains("LD/ST rel to"))
    member this.countersCPushPop          = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> cat.Contains("Stack push/pop"))
    member this.countersCMov              = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> cat.Contains("Register moves"))
    member this.countersCOthers           = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> not (cat.Contains("LD/ST rel to")) && not (cat.Contains("Stack push/pop")) && not (cat.Contains("Register moves")))
    member this.countersCTotal            = this.countersCPushPop + this.countersCMov + this.countersCLoadStore + this.countersCOthers;

    member this.countersAOTLoadStore      = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("LD/ST rel to") && not (cat.Contains("LD/ST rel to X")))
    member this.countersAOTPushPopInt     = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Stack push/pop"))
    member this.countersAOTPushPopRef     = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("LD/ST rel to X"))
    member this.countersAOTPushPop        = this.countersAOTPushPopInt + this.countersAOTPushPopRef
    member this.countersAOTMov            = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Register moves"))
    member this.countersAOTOthers         = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> not (cat.Contains("LD/ST rel to")) && not (cat.Contains("Stack push/pop")) && not (cat.Contains("Register moves")))
    member this.countersAOTTotal          = this.countersAOTPushPop + this.countersAOTMov + this.countersAOTLoadStore + this.countersAOTOthers;

    member this.countersOverheadLoadStore = this.countersAOTLoadStore - this.countersCLoadStore;
    member this.countersOverheadPushPop   = this.countersAOTPushPop - this.countersCPushPop;
    member this.countersOverheadMov       = this.countersAOTMov - this.countersCMov;
    member this.countersOverheadOthers    = this.countersAOTOthers - this.countersCOthers;
    member this.countersOverheadTotal     = this.countersAOTTotal - this.countersCTotal;

    member this.executedCyclesAOT         = this.countersAOTTotal.cycles
    member this.executedCyclesC           = this.countersCTotal.cycles
    member this.maxJvmStackInBytes        =
        this.jvmInstructions
        |> List.map (fun jvm -> jvm.djDebugData.stackSizeBefore)
        |> List.max;
    member this.avgJvmStackInBytes        =
        this.jvmInstructions 
        |> List.fold (fun (accCnt, accSum) jvm -> (accCnt+jvm.counters.executions, accSum+(jvm.counters.executions*jvm.djDebugData.stackSizeBefore))) (0, 0)
        |> (fun (cnt, sum) -> float sum / float cnt)
    member this.avgJvmStackChangeInBytes  =
        this.jvmInstructions 
        |> List.map (fun jvm -> (jvm.counters.executions, jvm.counters.executions * (abs (jvm.djDebugData.stackSizeBefore - jvm.djDebugData.stackSizeAfter))))
        |> List.fold (fun (accCnt, accSum) (jvmCnt, jvmSum) -> (accCnt+jvmCnt, accSum+jvmSum)) (0, 0)
        |> (fun (cnt, sum) -> float sum / float cnt)

