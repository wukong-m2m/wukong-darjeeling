#r "binaries/FSharp.Data/FSharp.Data.dll"
#r "binaries/fspickler.1.5.2/lib/net45/FsPickler.dll"

#load "Datatypes.fsx"
#load "Helpers.fsx"
#load "AVR.fsx"
#load "JVM.fsx"

open System
open System.IO
open System.Linq
open System.Text.RegularExpressions
open System.Runtime.Serialization
open FSharp.Data
open Nessos.FsPickler
open Datatypes
open Helpers

type RtcdataXml = XmlProvider<"rtcdata-example.xml", Global=true>
type Rtcdata = RtcdataXml.Methods
type MethodImpl = RtcdataXml.MethodImpl

let countersToString totalCycles totalBytes (counters : ExecCounters) =
    // String.Format("cyc:{0,8} {1,5:0.0}% {2,5:0.0}%C exe:{3,8}  avg:{4,5:0.0} byt:{5,5} {6,5:0.0}% {7,5:0.0}%C",
    String.Format("{0,11:n0} {1,5:0.0}% {2,9:n0} {3,9:0.0} | {4,5} {5,5:0.0}%",
                  counters.cycles,
                  100.0 * float (counters.cycles) / float totalCycles,
                  counters.executions,
                  counters.average,
                  counters.size,
                  100.0 * float (counters.size) / float totalBytes)

let getLoops results =
  let jvmInstructions = results.jvmInstructions |> List.map (fun jvm -> jvm.jvm)
  let filteredInstructions = jvmInstructions |> List.filter (fun jvm -> jvm.text.Contains("MARKLOOP") || jvm.text.Contains("JVM_BRTARGET") || jvm.text.Contains("Branch target"))
  filteredInstructions |> List.map (fun jvm -> String.Format("{0} {1}\n\r", jvm.index, jvm.text)) |> List.fold (+) ""

let resultsToString (results : SimulationResults) =
    let totalCyclesAOTJava = results.countersAOTTotal.cycles
    let totalCyclesNativeC = results.countersCTotal.cycles
    let totalBytesAOTJava = results.countersAOTTotal.size
    let totalBytesNativeC = results.countersCTotal.size
    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.00}", float cycles1 / float cycles2)
    let stackToString stack =
        String.Join(",", stack |> List.map (fun el -> el.datatype |> StackDatatypeToString))

    let countersHeaderString = "cycles                    exec   avg | bytes"
    let countersToString totalCycles totalBytes (counters : ExecCounters) =
        // String.Format("cyc:{0,8} {1,5:0.0}% {2,5:0.0}%C exe:{3,8}  avg:{4,5:0.0} byt:{5,5} {6,5:0.0}% {7,5:0.0}%C",
        String.Format("{0,8} {1,5:0.0}% {2,5:0.0}%C {3,8} {4,5:0.0} | {5,5} {6,5:0.0}% {7,5:0.0}%C",
                      counters.cycles,
                      100.0 * float (counters.cycles) / float totalCycles,
                      100.0 * float (counters.cycles) / float totalCyclesNativeC,
                      counters.executions,
                      counters.average,
                      counters.size,
                      100.0 * float (counters.size) / float totalBytes,
                      100.0 * float (counters.size) / float totalBytesNativeC)

    let resultJavaListingToString (result : ProcessedJvmInstruction) =
        String.Format("{0,-60}{1} {2}->{3}:{4}",
                      result.jvm.text,
                      countersToString totalCyclesAOTJava totalBytesAOTJava result.counters,
                      result.djDebugData.stackSizeBefore,
                      result.djDebugData.stackSizeAfter,
                      stackToString result.djDebugData.stackAfter)

    let resultsAvrToString (avr : ResultAvr) =
        let avrInstOption2Text = function
            | Some (x : AvrInstruction)
                -> String.Format("0x{0,6:X6}: {1,-15}", x.address, x.text)
            | None -> "" in
        String.Format("        {0,-15} -> {1,-30} {2,10} {3,23}",
                                                      avr.unopt.text,
                                                      avrInstOption2Text avr.opt,
                                                      avr.counters.cycles,
                                                      avr.counters.executions)

    let nativeCInstructionToString ((inst, counters) : AvrInstruction*ExecCounters) =
        String.Format("0x{0,6:X6}: {1}    {2}", inst.address, (countersToString totalCyclesNativeC totalBytesNativeC counters), inst.text)
    let opcodeResultsToString totalCycles totalBytes opcodeResults =
            opcodeResults
            |> List.map (fun (category, opcode, counters)
                             ->  String.Format("{0,-20}{1,-20} {2}",
                                               category,
                                               opcode,
                                               (countersToString totalCycles totalBytes counters)))
    let categoryResultsToString totalCycles totalBytes categoryResults =
            categoryResults
            |> List.map (fun (category, counters)
                             -> String.Format("{0,-40} {1}",
                                              category,
                                              (countersToString totalCycles totalBytes counters)))

    let testResultAOT = if results.passedTestAOT then "PASSED" else "FAILED"
    let testResultJAVA = if results.passedTestJava then "PASSED" else "FAILED"
    let sb = new Text.StringBuilder(10000000)
    let addLn s =
      sb.AppendLine(s) |> ignore
    addLn ("================== " + results.benchmark + ": AOT " + testResultAOT + ", Java " + testResultJAVA + " ================== ============================ CODE SIZE ===============================")
    addLn (String.Format ("--- STACK max                               {0,14}             --- CODE SIZE   Native C                    {1,14}", results.maxJvmStackInBytes, results.codesizeC))
    addLn (String.Format ("          avg/executed jvm                  {0,14:000.00}                             AOT                         {1,14}", results.avgJvmStackInBytes, results.codesizeAOT))
    addLn (String.Format ("          avg change/exec jvm               {0,14:000.00}                             Java total                  {1,14}", results.avgJvmStackChangeInBytes, results.codesizeJava))
    addLn (String.Format ("============================ STOPWATCHES =============================                      branch count           {0,14}",results.codesizeJavaBranchCount))
    addLn (String.Format ("--- STOPWATCHES Native C                    {0,14}                                  branch target count    {1,14}", results.cyclesStopwatchC, results.codesizeJavaBranchTargetCount))
    addLn (String.Format ("                AOT                         {0,14}                                  markloop count         {1,14}", results.cyclesStopwatchAOT, results.codesizeJavaMarkloopCount))
    addLn (String.Format ("                JAVA                        {0,14}                                  markloop size          {1,14}", results.cyclesStopwatchJava, results.codesizeJavaMarkloopTotalSize))
    addLn (String.Format ("                AOT/C                       {0,14}                                  total-branch overhead  {1,14}", cyclesToSlowdown results.cyclesStopwatchAOT results.cyclesStopwatchC, results.codesizeJavaWithoutBranchOverhead))
    addLn (String.Format ("                Java/C                      {0,14}                                  total-br/mloop overh.  {1,14}", cyclesToSlowdown results.cyclesStopwatchJava results.cyclesStopwatchC, results.codesizeJavaWithoutBranchMarkloopOverhead))
    addLn (String.Format ("                Java/AOT                    {0,14}                             AOT/C                       {1,14}", cyclesToSlowdown results.cyclesStopwatchJava results.cyclesStopwatchAOT, cyclesToSlowdown results.codesizeAOT results.codesizeC))
    addLn (String.Format ("                                                                                       AOT/Java                    {0,14}", cyclesToSlowdown results.codesizeAOT results.codesizeJava))
    addLn ("=============================================================== MAIN COUNTERS ===============================================================")
    addLn (String.Format ("--- NAT.C    Cycles                      {0,12} (stopwatch {1}, ratio {2}) (difference probably caused by interrupts)", results.executedCyclesC, results.cyclesStopwatchC, (cyclesToSlowdown results.cyclesStopwatchC results.executedCyclesC)))
    addLn (String.Format ("             Bytes                       {0,12} (address range {1}, ratio {2}) (off by 2 expected for methods ending in JMP)", totalBytesNativeC, results.codesizeC, (cyclesToSlowdown results.codesizeC totalBytesNativeC)))
    addLn ("                                           " + countersHeaderString)
    addLn (String.Format ("             Total                       {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCTotal)))
    addLn (String.Format ("              load/store                 {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCLoadStore)))
    addLn (String.Format ("              push/pop int               {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCPushPop)))
    addLn (String.Format ("              mov(w)                     {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCMov)))
    addLn (String.Format ("              others                     {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCOthers)))
    addLn ("")
    addLn (String.Format ("--- AOT      Cycles                      {0,12} (stopwatch {1}, ratio {2}) (difference probably caused by interrupts)", results.executedCyclesAOT, results.cyclesStopwatchAOT, (cyclesToSlowdown results.cyclesStopwatchAOT results.executedCyclesAOT)))
    addLn (String.Format ("             Bytes                       {0,12} (methodImpl.AvrMethodSize {1}, ratio {2})", totalBytesAOTJava, results.codesizeAOT, (cyclesToSlowdown results.codesizeAOT totalBytesAOTJava)))
    addLn ("                                           " + countersHeaderString)
    addLn (String.Format ("             Total                       {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTTotal)))
    addLn (String.Format ("              load/store                 {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTLoadStore)))
    addLn (String.Format ("              push/pop int               {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTPushPopInt)))
    addLn (String.Format ("              push/pop ref               {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTPushPopRef)))
    addLn (String.Format ("              mov(w)                     {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTMov)))
    addLn (String.Format ("              others                     {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTOthers)))
    addLn ("")
    addLn ("                                           " + countersHeaderString)
    addLn (String.Format ("--- OVERHEAD Total                       {0,14}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersOverheadTotal)))
    addLn (String.Format ("              load/store                 {0,14}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersOverheadLoadStore)))
    addLn (String.Format ("              pushpop                    {0,14}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersOverheadPushPop)))
    addLn (String.Format ("              mov(w)                     {0,14}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersOverheadMov)))
    addLn (String.Format ("              others                     {0,14}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersOverheadOthers)))
    addLn ("=============================================================== DETAILED COUNTERS ===========================================================")
    addLn ("--- SUMMED: PER JVM CATEGORY               " + countersHeaderString)
    categoryResultsToString totalCyclesAOTJava totalBytesAOTJava results.countersPerJvmOpcodeCategoryAOTJava
      |> List.iter addLn
    addLn ("")
    addLn ("--- SUMMED: PER AVR CATEGORY (Java AOT)    " + countersHeaderString)
    categoryResultsToString totalCyclesAOTJava totalBytesAOTJava results.countersPerAvrOpcodeCategoryAOTJava
      |> List.iter addLn
    addLn ("")
    addLn ("--- SUMMED: PER AVR CATEGORY (NATIVE C)    " + countersHeaderString)
    categoryResultsToString totalCyclesNativeC totalBytesNativeC results.countersPerAvrOpcodeCategoryNativeC
      |> List.iter addLn
    addLn ("")
    addLn ("--- SUMMED: PER JVM OPCODE                 " + countersHeaderString)
    opcodeResultsToString totalCyclesAOTJava totalBytesAOTJava results.countersPerJvmOpcodeAOTJava
      |> List.iter addLn
    addLn ("")
    addLn ("--- SUMMED: PER AVR OPCODE (Java AOT)      " + countersHeaderString)
    opcodeResultsToString totalCyclesAOTJava totalBytesAOTJava results.countersPerAvrOpcodeAOTJava
      |> List.iter addLn
    addLn ("")
    addLn ("--- SUMMED: PER AVR OPCODE (NATIVE C)      " + countersHeaderString)
    opcodeResultsToString totalCyclesNativeC totalBytesNativeC results.countersPerAvrOpcodeNativeC
      |> List.iter addLn
    addLn ("")
    addLn ("=============================================================== LISTINGS ====================================================================")
    addLn ("")
    addLn ("--- LISTING: NATIVE C AVR")
    addLn ("            " + countersHeaderString)
    results.nativeCInstructions
      |> List.map nativeCInstructionToString
      |> List.iter addLn
    addLn ("")
    addLn ("--- LISTING: ONLY JVM                                         " + countersHeaderString)
    results.jvmInstructions
      |> List.map resultJavaListingToString
      |> List.iter addLn
    addLn ("")
    addLn ("--- LISTING: JVM LOOPS")
    addLn (getLoops results)
    addLn ("")
    addLn ("--- LISTING: ONLY OPTIMISED AVR                               " + countersHeaderString)
    results.jvmInstructions
      |> List.map (fun r -> (r |> resultJavaListingToString) :: (r.avr |> List.filter (fun avr -> avr.opt.IsSome) |> List.map resultsAvrToString))
      |> List.concat
      |> List.iter addLn
    addLn ("")
    addLn ("--- LISTING: COMPLETE LISTING")
    results.jvmInstructions
      |> List.map (fun r -> (r |> resultJavaListingToString) :: (r.avr |> List.map resultsAvrToString))
      |> List.concat
      |> List.iter addLn
    let result = (sb.ToString())
    result

let resultsToSummaryListEntry (impl : MethodImpl) (results : SimulationResults) =
    let totalCycles =
        results.jvmInstructions |> List.map (fun j -> j.counters.cycles)
                                |> List.reduce (+)
    let ownCycles =
        results.jvmInstructions |> List.filter (fun j -> not (j.jvm.text.StartsWith("JVM_INVOKE")))
                                |> List.map (fun j -> j.counters.cycles)
                                |> List.reduce (+)
    let numberOfExecutions =
        let firstJvmInstruction = results.jvmInstructions |> List.head
        firstJvmInstruction.counters.executions
    let methodName = getMethodNameFromImpl impl
    let entry = String.Format("{0,11:n0} {1,11:n0} {2,7:n0}   {3}",
                    totalCycles,
                    ownCycles,
                    numberOfExecutions,
                    methodName)
    entry

let resultsToCalledMethodsList (results : SimulationResults) =
    let totalCyclesAOTJava = results.countersAOTTotal.cycles
    let totalBytesAOTJava = results.countersAOTTotal.size

    let nonInvokeCounters = results.jvmInstructions |> List.filter (fun j -> not (j.jvm.text.StartsWith("JVM_INVOKE")))
                                                    |> List.map (fun j -> j.counters)
                                                    |> List.fold (+) ExecCounters.empty
    let totalCounters = results.jvmInstructions |> List.map (fun j -> j.counters)
                                                |> List.fold (+) ExecCounters.empty
    let groupedInvokes =
        results.jvmInstructions |> List.filter (fun j -> (j.jvm.text.StartsWith("JVM_INVOKE")))
                                |> List.map (fun j -> (getMethodNameFromFullName j.jvm.text, j.counters)) // This will work for now, since we just take the name from the second
                                |> groupFold fst snd (+) ExecCounters.empty
    let combined = ("----own----", nonInvokeCounters) :: groupedInvokes @ [ ("---total---", totalCounters) ]

    combined |> List.map (fun (target, counters)
                                -> String.Format("{0,53} : {1}\n\r",
                                                 countersToString totalCyclesAOTJava totalBytesAOTJava counters,
                                                 target))
             |> List.fold (+) ""

let resultsToProfiledText (impl : MethodImpl) (results : SimulationResults) =
    let totalCyclesAOTJava = results.countersAOTTotal.cycles
    let totalCyclesNativeC = results.countersCTotal.cycles
    let totalBytesAOTJava = results.countersAOTTotal.size
    let totalBytesNativeC = results.countersCTotal.size
    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.00}", float cycles1 / float cycles2)
    let stackToString stack =
        String.Join(",", stack |> List.map (fun el -> el.datatype |> StackDatatypeToString))

    let countersHeaderString = "cycles                    exec   avg | bytes"

    let resultJavaListingToString (result : ProcessedJvmInstruction) =
        let jvmTextFirstPart (text : String) =
            match text.StartsWith ("JVM_INVOKE") with
            | false -> text
            | true -> text.Substring(0, text.IndexOf(" "))
        let jvmTextSecondPart (text : String) =
            match text.StartsWith ("JVM_INVOKE") with
            | false -> ""
            | true -> getMethodNameFromFullName text
        String.Format("{0,-60}{1,50} {2}",
                      jvmTextFirstPart result.jvm.text,
                      countersToString totalCyclesAOTJava totalBytesAOTJava result.counters,
                      jvmTextSecondPart result.jvm.text)

    let resultsAvrToString (avr : ResultAvr) =
        let avrInstOption2Text = function
            | Some (x : AvrInstruction)
                -> String.Format("0x{0,6:X6}: {1,-15}", x.address, x.text)
            | None -> "" in
        String.Format("        {0,-15} -> {1,-30} {2,10} {3,23}",
                                                      avr.unopt.text,
                                                      avrInstOption2Text avr.opt,
                                                      avr.counters.cycles,
                                                      avr.counters.executions)

    let sb = new Text.StringBuilder(10000000)
    let addLn s =
      sb.AppendLine(s) |> ignore

    addLn ("----------------------------------------------------------")
    addLn ((getMethodNameFromImpl impl))
    addLn ("----------------------------------------------------------")

    addLn (resultsToCalledMethodsList results)

    addLn ("")
    addLn ("--- ONLY JVM                                               " + countersHeaderString)
    results.jvmInstructions
      |> List.map resultJavaListingToString
      |> List.iter addLn
    addLn ("")
    addLn ("--- JVM + AVR                                              " + countersHeaderString)
    results.jvmInstructions
      |> List.map (fun r -> (r |> resultJavaListingToString) :: (r.avr |> List.filter (fun avr -> avr.opt.IsSome) |> List.map resultsAvrToString))
      |> List.concat
      |> List.iter addLn
    addLn ("")
    addLn ("")
    let result = (sb.ToString())
    result

let multipleResultsToProfileReport (allMethodResults : (MethodImpl * SimulationResults) list) =
    let opcodeResultsToString totalCycles totalBytes opcodeResults =
            opcodeResults
            |> List.map (fun (category, opcode, counters)
                             ->  String.Format("{0,-20}{1,-20} {2}",
                                               category,
                                               opcode,
                                               (countersToString totalCycles totalBytes counters)))
    let categoryResultsToString totalCycles totalBytes categoryResults =
            categoryResults
            |> List.map (fun (category, counters)
                             -> String.Format("{0,-40} {1}",
                                              category,
                                              (countersToString totalCycles totalBytes counters)))

    let summaryListHeader = "  cyc.total     cyc.own    exec   method"

    let sb = new Text.StringBuilder(10000000)
    let addLn s =
      sb.AppendLine(s) |> ignore


    addLn("========================================SUMMARY========================================")
    addLn("")
    addLn("")
    addLn(summaryListHeader)
    allMethodResults |> List.map (fun (methodImpl, results) -> (resultsToSummaryListEntry methodImpl results))
                     |> List.iter addLn
    addLn("")
    addLn("")
    addLn("====================================INVOKE OVERVIEW====================================")
    addLn("")
    addLn("")
    allMethodResults |> List.map (fun (methodImpl, results) -> String.Format("{0}\n\r{1}", (getMethodNameFromImpl methodImpl), (resultsToCalledMethodsList results)))
                     |> List.iter addLn
    addLn("")
    addLn("")
    addLn("==================================== MAIN COUNTERS ====================================")
    addLn("")
    addLn("")

    let countersAOTTotal      = allMethodResults |> List.map (fun (i,r) -> r.countersAOTTotal)      |> List.fold (+) ExecCounters.empty
    let countersAOTLoadStore  = allMethodResults |> List.map (fun (i,r) -> r.countersAOTLoadStore)  |> List.fold (+) ExecCounters.empty
    let countersAOTPushPopInt = allMethodResults |> List.map (fun (i,r) -> r.countersAOTPushPopInt) |> List.fold (+) ExecCounters.empty
    let countersAOTPushPopRef = allMethodResults |> List.map (fun (i,r) -> r.countersAOTPushPopRef) |> List.fold (+) ExecCounters.empty
    let countersAOTMov        = allMethodResults |> List.map (fun (i,r) -> r.countersAOTMov)        |> List.fold (+) ExecCounters.empty
    let countersAOTOthers     = allMethodResults |> List.map (fun (i,r) -> r.countersAOTOthers)     |> List.fold (+) ExecCounters.empty

    addLn(               "             Note that 'others' includes INVOKEs, so there's a lot of double counting right now.")
    addLn("")
    addLn(String.Format ("             Total                       {0}", (countersToString countersAOTTotal.cycles countersAOTTotal.size countersAOTTotal)))
    addLn(String.Format ("              load/store                 {0}", (countersToString countersAOTTotal.cycles countersAOTTotal.size countersAOTLoadStore)))
    addLn(String.Format ("              push/pop int               {0}", (countersToString countersAOTTotal.cycles countersAOTTotal.size countersAOTPushPopInt)))
    addLn(String.Format ("              push/pop ref               {0}", (countersToString countersAOTTotal.cycles countersAOTTotal.size countersAOTPushPopRef)))
    addLn(String.Format ("              mov(w)                     {0}", (countersToString countersAOTTotal.cycles countersAOTTotal.size countersAOTMov)))
    addLn(String.Format ("              others                     {0}", (countersToString countersAOTTotal.cycles countersAOTTotal.size countersAOTOthers)))

    addLn("")
    addLn("")
    addLn("================================== DETAILED COUNTERS ==================================")
    addLn("")
    addLn("")
    addLn(" Note that the percentages aren't accurate, since they're measured against the wrong total (see MAIN COUNTERS).")
    addLn("")

    let countersPerJvmOpcodeCategoryAOTJava =
      allMethodResults
        |> List.collect (fun (i,r) -> r.countersPerJvmOpcodeCategoryAOTJava)
        |> groupFold fst snd  (+) ExecCounters.empty
    let countersPerJvmOpcodeAOTJava =
      allMethodResults
        |> List.collect (fun (i,r) -> r.countersPerJvmOpcodeAOTJava)
        |> groupFold (fun (cat, opcode, cnt) -> (cat, opcode)) (fun (cat, opcode, cnt) -> cnt) (+) ExecCounters.empty
        |> List.map (fun (catOpcode, cnt) -> ((fst catOpcode), (snd catOpcode), cnt))


    addLn ("--- SUMMED: PER JVM CATEGORY               ")
    categoryResultsToString countersAOTTotal.cycles countersAOTTotal.size countersPerJvmOpcodeCategoryAOTJava
      |> List.iter addLn
    addLn ("")
    addLn ("--- SUMMED: PER JVM OPCODE                 ")
    opcodeResultsToString countersAOTTotal.cycles countersAOTTotal.size countersPerJvmOpcodeAOTJava
      |> List.iter addLn

    addLn("")
    addLn("")
    addLn("====================================DETAILED TRACES====================================")
    addLn("")
    addLn("")
    allMethodResults |> List.map (fun (methodImpl, results) -> (resultsToProfiledText methodImpl results))
                     |> List.iter addLn
    let result = (sb.ToString())
    result
