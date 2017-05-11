#r "binaries/FSharp.Data/FSharp.Data.dll"

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
open Datatypes
open Helpers

type RtcdataXml = XmlProvider<"rtcdata-example.xml", Global=true>
type Rtcdata = RtcdataXml.Methods
type MethodImpl = RtcdataXml.MethodImpl

let resultsToString (results : SimulationResults) =
    let sb = new Text.StringBuilder(10000000)
    let addLn s =
      sb.AppendLine(s) |> ignore

    let asPercentage a b =
      100.0 * float a / float b
    let totalCyclesAOTJava = results.countersAOTTotal.cycles
    let totalCyclesNativeC = results.countersCTotal.cycles
    let totalCyclesOverhead = results.countersOverheadTotal.cycles
    let totalBytesAOTJava = results.countersAOTTotal.size
    let totalBytesNativeC = results.countersCTotal.size
    let totalBytesOverhead = results.countersOverheadTotal.size

    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.000}", float cycles1 / float cycles2)
    let stackToString stack =
        String.Join(",", stack |> List.map (fun el -> el.datatype |> StackDatatypeToString))

    let countersHeaderString = "  cycles                      exec   avg | bytes"
    let countersToString totalCycles totalBytes (counters : ExecCounters) =
        // String.Format("cyc:{0,8} {1,5:0.0}% {2,5:0.0}%C exe:{3,8}  avg:{4,5:0.0} byt:{5,5} {6,5:0.0}% {7,5:0.0}%C",
        String.Format("{0,10} {1,5:0.0}% {2,5:0.0}%C {3,10} {4,5:0.0} | {5,5} {6,5:0.0}% {7,5:0.0}%C",
                      counters.cycles,
                      asPercentage counters.cycles totalCycles,
                      asPercentage counters.cycles totalCyclesNativeC,
                      counters.executions,
                      counters.average,
                      counters.size,
                      asPercentage counters.size totalBytes,
                      asPercentage counters.size totalBytesNativeC)

    let countersToStringInclSubroutines totalCycles totalBytes (counters : ExecCounters) =
        // String.Format("cyc:{0,8} {1,5:0.0}% {2,5:0.0}%C exe:{3,8}  avg:{4,5:0.0} byt:{5,5} {6,5:0.0}% {7,5:0.0}%C",
        String.Format("{0,11:n0} {1,5:0.0}% {2,9:n0} {3,9:0.0} | {4,5} {5,5:0.0}%",
                      counters.cyclesInclSubroutine,
                      100.0 * float (counters.cyclesInclSubroutine) / float totalCycles,
                      counters.executions,
                      counters.average,
                      counters.size,
                      100.0 * float (counters.size) / float totalBytes)
        
    let resultJavaListingToString totalCycles totalBytes (result : ProcessedJvmInstruction) =
        let (instruction, invokeTarget) =
          match (result.jvm.isInvoke) with
          | true  -> (result.jvm.instructionOnly, "      TARGET " +  result.jvm.instructionDetails)
          | false -> (result.jvm.text, "")
        String.Format("{0,-60}{1} {2}->{3}:{4} {5}",
                      instruction,
                      countersToString totalCyclesAOTJava totalBytesAOTJava result.counters,
                      result.djDebugData.stackSizeBefore,
                      result.djDebugData.stackSizeAfter,
                      stackToString result.djDebugData.stackAfter,
                      invokeTarget)

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

    let nativeCInstructionToString totalCycles totalBytes ((inst, counters) : AvrInstruction*ExecCounters) =
        String.Format("0x{0,6:X6}: {1}    {2}", inst.address, (countersToString totalCycles totalBytes counters), inst.text)
    let opcodeResultsToString totalCycles totalBytes opcodeResults =
            opcodeResults
            |> List.sortBy (fun (category, opcode, counters) -> (category+opcode))
            |> List.map (fun (category, opcode, counters)
                             ->  String.Format("{0,-20}{1,-20} {2}",
                                               category,
                                               opcode,
                                               (countersToString totalCycles totalBytes counters)))
    let categoryResultsToString totalCycles totalBytes categoryResults =
            categoryResults
            |> List.sortBy (fun (category, counters) -> category)
            |> List.map (fun (category, counters)
                             -> String.Format("{0,-40} {1}",
                                              category,
                                              (countersToString totalCycles totalBytes counters)))


    let addJvmMethodCalledMethodsList (jvmMethod : JvmMethod) =
        addLn jvmMethod.name

        let totalCyclesAOTJava = jvmMethod.countersAOTTotal.cyclesInclSubroutine
        let totalBytesAOTJava = jvmMethod.countersAOTTotal.size

        let numberOfExecutions =
            let firstJvmInstruction = jvmMethod.instructions |> List.head
            firstJvmInstruction.counters.executions

        let nonInvokeCounters =
          let sumOfAllNonInvokeCounters = jvmMethod.instructions |> List.filter (fun j -> not (j.jvm.text.StartsWith("JVM_INVOKE")))
                                                                  |> List.map (fun j -> j.counters)
                                                                  |> List.fold (+) ExecCounters.Zero
          { sumOfAllNonInvokeCounters with executions = numberOfExecutions }

        let totalCounters = 
          let sumOfAllCounters = jvmMethod.instructions |> List.map (fun j -> j.counters)
                                                         |> List.fold (+) ExecCounters.Zero
          { sumOfAllCounters with executions = numberOfExecutions }

        let groupedInvokes =
            jvmMethod.instructions |> List.filter (fun j -> (j.jvm.text.StartsWith("JVM_INVOKE")))
                                    |> List.map (fun j -> (getClassAndMethodNameFromFullName j.jvm.text, j.counters)) // This will work for now, since we just take the name from the second
                                    |> groupFold fst snd (+) ExecCounters.Zero
        let combined = ("---total---", totalCounters) :: ("----own----", nonInvokeCounters) :: groupedInvokes

        combined |> List.map (fun (target, counters)
                                    -> String.Format("{0,53} : {1}",
                                                     countersToStringInclSubroutines totalCyclesAOTJava totalBytesAOTJava counters,
                                                     target))
                 |> List.iter addLn
        addLn ""

    let addJvmMethodDetails (jvmMethod : JvmMethod) =
      let totalCyclesAOTJava = jvmMethod.countersAOTTotal.cycles
      let totalBytesAOTJava = jvmMethod.countersAOTTotal.size

      addLn ("--------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM -----------")
      addLn ("                                                              " + jvmMethod.name)
      addLn ("--------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM ----------- JVM -----------")
      addLn ("")
      addJvmMethodCalledMethodsList jvmMethod
      addLn ("--- SUMMED: PER JVM CATEGORY               " + jvmMethod.name)
      addLn ("                                           " + countersHeaderString)
      categoryResultsToString totalCyclesAOTJava totalBytesAOTJava jvmMethod.countersPerJvmOpcodeCategoryAOTJava
        |> List.iter addLn
      addLn ("")
      addLn ("--- SUMMED: PER AVR CATEGORY (Java AOT)    " + jvmMethod.name)
      addLn ("                                           " + countersHeaderString)
      categoryResultsToString totalCyclesAOTJava totalBytesAOTJava jvmMethod.countersPerAvrOpcodeCategoryAOTJava
        |> List.iter addLn
      addLn ("")
      addLn ("--- SUMMED: PER JVM OPCODE                 " + jvmMethod.name)
      addLn ("                                           " + countersHeaderString)
      opcodeResultsToString totalCyclesAOTJava totalBytesAOTJava jvmMethod.countersPerJvmOpcodeAOTJava
        |> List.iter addLn
      addLn ("")
      addLn ("--- SUMMED: PER AVR OPCODE (Java AOT)      " + jvmMethod.name)
      addLn ("                                           " + countersHeaderString)
      opcodeResultsToString totalCyclesAOTJava totalBytesAOTJava jvmMethod.countersPerAvrOpcodeAOTJava
        |> List.iter addLn
      addLn ("")

      addLn ("--- LISTING: JVM LOOPS " + jvmMethod.name)
      let jvmInstructions = jvmMethod.instructions |> List.map (fun jvm -> jvm.jvm)
      let filteredInstructions = jvmInstructions |> List.filter (fun jvm -> jvm.text.Contains("MARKLOOP") || jvm.text.Contains("JVM_BRTARGET") || jvm.text.Contains("Branch target"))
      filteredInstructions |> List.map (fun jvm -> String.Format("{0} {1}", jvm.index, jvm.text))
                           |> List.iter addLn
      addLn ("")
      addLn ("--- LISTING: ONLY JVM                                         " + jvmMethod.name)
      addLn ("                                                              " + countersHeaderString)
      jvmMethod.instructions
        |> List.map (resultJavaListingToString totalCyclesAOTJava totalBytesAOTJava)
        |> List.iter addLn
      addLn ("")

      addLn ("--- LISTING: ONLY OPTIMISED AVR                               " + countersHeaderString)
      jvmMethod.instructions
        |> List.map (fun r -> (r |> resultJavaListingToString totalCyclesAOTJava totalBytesAOTJava) :: (r.avr |> List.filter (fun avr -> avr.opt.IsSome) |> List.map resultsAvrToString))
        |> List.concat
        |> List.iter addLn
      addLn ("")

    let addJvmFullListing (jvmMethod : JvmMethod) =
      let totalCyclesAOTJava = jvmMethod.countersAOTTotal.cycles
      let totalBytesAOTJava = jvmMethod.countersAOTTotal.size

      addLn ("--- LISTING: COMPLETE LISTING                                 " + countersHeaderString)
      jvmMethod.instructions
        |> List.map (fun r -> (r |> resultJavaListingToString totalCyclesAOTJava totalBytesAOTJava) :: (r.avr |> List.map resultsAvrToString))
        |> List.concat
        |> List.iter addLn


    let addCFunctionCalledFunctionsList (cFunction : CFunction) =
        addLn cFunction.name

        let totalCyclesC = cFunction.countersCTotal.cyclesInclSubroutine
        let totalBytesC = cFunction.countersCTotal.size

        let numberOfExecutions =
            let firstAvrInstructionCounters = cFunction.instructions |> List.head |> snd
            firstAvrInstructionCounters.executions

        let nonInvokeCounters =
          let sumOfAllNonInvokeCounters = cFunction.instructions |> List.filter (fun (avr,cnt) -> not (avr.text.StartsWith("call")))
                                                                 |> List.map (fun (avr,cnt) -> cnt)
                                                                 |> List.fold (+) ExecCounters.Zero
          { sumOfAllNonInvokeCounters with executions = numberOfExecutions }

        let totalCounters = 
          let sumOfAllCounters = cFunction.instructions |> List.map (fun (avr,cnt) -> cnt)
                                                        |> List.fold (+) ExecCounters.Zero
          { sumOfAllCounters with executions = numberOfExecutions }

        let groupedInvokes =
            cFunction.instructions |> List.filter (fun (avr,cnt) -> (avr.text.StartsWith("call")))
                                   |> List.map (fun (avr,cnt) -> ((avr.text.Substring(21)), cnt)) // This will work for now, since we just take the name from the second
                                   |> groupFold fst snd (+) ExecCounters.Zero
        let combined = ("---total---", totalCounters) :: ("----own----", nonInvokeCounters) :: groupedInvokes

        combined |> List.map (fun (target, counters)
                                    -> String.Format("{0,53} : {1}",
                                                     countersToStringInclSubroutines totalCyclesC totalBytesC counters,
                                                     target))
                 |> List.iter addLn
        addLn ""

    let addCFunctionDetails (cFunction : CFunction) =
      let totalCyclesNativeC = cFunction.countersCTotal.cycles
      let totalBytesNativeC = cFunction.countersCTotal.size

      addLn ("---------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------")
      addLn ("                                                              " + cFunction.name)
      addLn ("---------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------  C  -----------")
      addLn ("")
      addCFunctionCalledFunctionsList cFunction
      addLn ("--- SUMMED: PER AVR CATEGORY (NATIVE C)    " + cFunction.name)
      addLn ("                                           " + countersHeaderString)
      categoryResultsToString totalCyclesNativeC totalBytesNativeC cFunction.countersPerAvrOpcodeCategoryNativeC
        |> List.iter addLn
      addLn ("")
      addLn ("--- SUMMED: PER AVR OPCODE (NATIVE C)      " + cFunction.name)
      addLn ("                                           " + countersHeaderString)
      opcodeResultsToString totalCyclesNativeC totalBytesNativeC cFunction.countersPerAvrOpcodeNativeC
        |> List.iter addLn
      addLn ("")
      addLn ("--- LISTING (NATIVE C): " + cFunction.name)
      addLn ("            " + countersHeaderString)
      cFunction.instructions
        |> List.map (nativeCInstructionToString totalCyclesNativeC totalBytesNativeC)
        |> List.iter addLn
      addLn ("")

    let addCyclesForSymbols (symbolsAndCounters : (string * ExecCounters) list) =
      symbolsAndCounters |> List.filter (fun (_, cnt) -> cnt.cycles > 0)
                         |> List.iter (fun (name, cnt) -> addLn(String.Format("{0,12} cyc, {1,12} sub, {2,12} total in {3}", cnt.cycles, cnt.cyclesInclSubroutine, (cnt.cycles + cnt.cyclesInclSubroutine), name)))


    let testResultAOT = if results.passedTestAOT then "PASSED" else "FAILED"
    addLn ("================== " + results.benchmark + ": AOT " + testResultAOT + "=============================== ============================ CODE SIZE ===============================")
    addLn (String.Format ("--- STACK max                               {0,14}             --- CODE SIZE   Native C                    {1,14}", results.maxJvmStackInBytes, results.codesizeC))
    addLn (String.Format ("          avg/executed jvm                  {0,14:000.00}                             AOT                         {1,14}", results.avgJvmStackInBytes, results.codesizeAOT))
    addLn (String.Format ("          avg change/exec jvm               {0,14:000.00}                             Java for AOT                {1,14}", results.avgJvmStackChangeInBytes, results.codesizeJavaForAOT))
    addLn (String.Format ("============================ STOPWATCHES =============================                       branch count          {0,14}", results.codesizeJavaBranchCount))
    addLn (String.Format ("--- STOPWATCHES Native C                    {0,14}                                   branch target count   {1,14}", results.cyclesStopwatchC, results.codesizeJavaBranchTargetCount))
    addLn (String.Format ("                AOT                         {0,14}                                   markloop count        {1,14}", results.cyclesStopwatchAOT, results.codesizeJavaMarkloopCount))
    addLn (String.Format ("                AOT/C                       {0,14}                                   markloop size         {1,14}", cyclesToSlowdown results.cyclesStopwatchAOT results.cyclesStopwatchC, results.codesizeJavaMarkloopTotalSize))
    addLn (String.Format ("                                                                                             total-branch overhead {0,14}", results.codesizeJavaWithoutBranchOverhead))
    addLn (String.Format ("                                                                                             total-br/mloop overh. {0,14}", results.codesizeJavaWithoutBranchMarkloopOverhead))
    addLn (String.Format ("                                                                                       Java (for interpreter)      {0,14}", results.codesizeJavaForInterpreter))
    addLn (String.Format ("                                                                                       AOT/C                       {0,14}", cyclesToSlowdown results.codesizeAOT results.codesizeC))
    addLn (String.Format ("                                                                                       AOT/Java                    {0,14}", cyclesToSlowdown results.codesizeAOT results.codesizeJavaForInterpreter))
    addLn ("=============================================================== MAIN COUNTERS ===============================================================")
    addLn (String.Format ("--- NAT.C    Bytes                       {0,12} (address range {1}, ratio {2}) (off by 2 expected for methods ending in JMP)", totalBytesNativeC, results.codesizeC, (cyclesToSlowdown results.codesizeC totalBytesNativeC)))
    addLn ("                                           " + countersHeaderString)
    addLn (String.Format ("             Total                       {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCTotal)))
    addLn (String.Format ("              load/store                 {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCLoadStore)))
    addLn (String.Format ("              push/pop int               {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCPushPop)))
    addLn (String.Format ("              mov(w)                     {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCMov)))
    addLn (String.Format ("              others                     {0}", (countersToString totalCyclesNativeC totalBytesNativeC results.countersCOthers)))
    addLn ("")
    addLn (String.Format ("--- AOT      Bytes                       {0,12} (methodImpl.AvrMethodSize {1}, ratio {2})", totalBytesAOTJava, results.codesizeAOT, (cyclesToSlowdown results.codesizeAOT totalBytesAOTJava)))
    addLn ("                                           " + countersHeaderString)
    addLn (String.Format ("             Total                       {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTTotal)))
    addLn (String.Format ("              load/store                 {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTLoadStore)))
    addLn (String.Format ("              push/pop int               {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTPushPopInt)))
    addLn (String.Format ("              push/pop ref               {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTPushPopRef)))
    addLn (String.Format ("              mov(w)                     {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTMov)))
    addLn (String.Format ("              spent in vm                {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTVM)))
    addLn (String.Format ("              others                     {0}", (countersToString totalCyclesAOTJava totalBytesAOTJava results.countersAOTOthers)))
    addLn ("")
    addLn ("                                           " + countersHeaderString)
    addLn (String.Format ("--- OVERHEAD Total                       {0,14}", (countersToString totalCyclesOverhead totalBytesOverhead results.countersOverheadTotal)))
    addLn (String.Format ("              load/store                 {0,14}", (countersToString totalCyclesOverhead totalBytesOverhead results.countersOverheadLoadStore)))
    addLn (String.Format ("              pushpop                    {0,14}", (countersToString totalCyclesOverhead totalBytesOverhead results.countersOverheadPushPop)))
    addLn (String.Format ("              mov(w)                     {0,14}", (countersToString totalCyclesOverhead totalBytesOverhead results.countersOverheadMov)))
    addLn (String.Format ("              spent in vm                {0,14}", (countersToString totalCyclesOverhead totalBytesOverhead results.countersOverheadVm)))
    addLn (String.Format ("              others                     {0,14}", (countersToString totalCyclesOverhead totalBytesOverhead results.countersOverheadOthers)))
    addLn ("")
    addLn (String.Format ("--- STOPWATCH / COUNTERS RATIO"))
    addLn (String.Format ("              C   stopwatch              {0,14}", results.cyclesStopwatchC))
    addLn (String.Format ("              C   total counters + timer {0,14} (={1,14}+{2,14})", results.countersCTotalPlusTimer.cycles, results.countersCTotal.cycles, results.countersCTimer.cycles))
    addLn (String.Format ("              C   ratio                  {0,14} (cause of difference unknown)", (cyclesToSlowdown results.cyclesStopwatchC results.countersCTotalPlusTimer.cycles)))
    addLn ("")
    addLn (String.Format ("              AOT stopwatch              {0,14}", results.cyclesStopwatchAOT))
    addLn (String.Format ("              AOT total counters + timer {0,14} (={1,14}+{2,14})", results.countersAOTTotalPlusTimer.cycles, results.countersAOTTotal.cycles, results.countersAOTTimer.cycles))
    addLn (String.Format ("              AOT ratio                  {0,14} (vm time calculated based on stopwatch, so this always matches now)", (cyclesToSlowdown results.cyclesStopwatchAOT results.countersAOTTotalPlusTimer.cycles)))
    addLn ("")
    addLn (String.Format ("--- EXECUTED JVM INSTRUCTIONS (executions, not cycles)"))
    addLn (String.Format (" Load/Store               {0,10}      {1,5:0.0}%", results.countersJVMLoadStore.executions, asPercentage results.countersJVMLoadStore.executions results.countersJVMTotal.executions))
    addLn (String.Format (" Constant load            {0,10}      {1,5:0.0}%", results.countersJVMConstantLoad.executions, asPercentage results.countersJVMConstantLoad.executions results.countersJVMTotal.executions))
    addLn (String.Format (" Processing               {0,10}      {1,5:0.0}%", results.countersJVMProcessing.executions, asPercentage results.countersJVMProcessing.executions results.countersJVMTotal.executions))
    addLn (String.Format ("     math                 {0,10}      {1,5:0.0}%", results.countersJVMProcessingMath.executions, asPercentage results.countersJVMProcessingMath.executions results.countersJVMTotal.executions))
    addLn (String.Format ("     bit shift            {0,10}      {1,5:0.0}%", results.countersJVMProcessingBitShift.executions, asPercentage results.countersJVMProcessingBitShift.executions results.countersJVMTotal.executions))
    addLn (String.Format ("     bit logic            {0,10}      {1,5:0.0}%", results.countersJVMProcessingBitLogic.executions, asPercentage results.countersJVMProcessingBitLogic.executions results.countersJVMTotal.executions))
    addLn (String.Format (" Branches                 {0,10}      {1,5:0.0}%", results.countersJVMBranches.executions, asPercentage results.countersJVMBranches.executions results.countersJVMTotal.executions))
    addLn (String.Format (" Others                   {0,10}      {1,5:0.0}%", results.countersJVMOthers.executions, asPercentage results.countersJVMOthers.executions results.countersJVMTotal.executions))
    addLn (String.Format (" Total                    {0,10}      {1,5:0.0}%", results.countersJVMTotal.executions, asPercentage results.countersJVMTotal.executions results.countersJVMTotal.executions))
    addLn ("")
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
    addLn ("============================================================= JVM LISTINGS ==================================================================")
    addLn ("")
    addLn ("------------------------------------------------------------ INVOKE OVERVIEW ----------------------------------------------------------------")
    addLn (" NOTE: cycles counter for method calls are timed at runtime.")
    addLn ("       this means time spent in interrupts is added to the current method,")
    addLn ("       so the totals printed here may be slightly higher than the total sum in the header. (which should be leading)")
    addLn ("")
    let sortedJvmMethodResults = results.jvmMethods |> List.sortBy (fun (jvmMethod) -> 0-jvmMethod.countersAOTTotal.cyclesInclSubroutine)
    sortedJvmMethodResults |> List.iter addJvmMethodCalledMethodsList
    addLn ("")
    sortedJvmMethodResults |> List.iter addJvmMethodDetails

    addLn ("============================================================== C LISTINGS ===================================================================")
    addLn ("")
    let sortedCFunctionResults = results.cFunctions |> List.sortBy (fun (cFunction) -> 0-cFunction.countersCTotal.cyclesInclSubroutine)
    addLn ("------------------------------------------------------------- CALL OVERVIEW -----------------------------------------------------------------")
    sortedCFunctionResults |> List.iter addCFunctionCalledFunctionsList
    addLn ("")
    sortedCFunctionResults |> List.iter addCFunctionDetails

    addLn ("")
    addLn ("============================================ COUNTERS FOR ALL SYMBOLS WHERE CYCLES>0 (JVM) ==================================================")
    addCyclesForSymbols results.jvmAllSymbolCounters
    addLn ("")

    addLn ("")
    addLn ("============================================ COUNTERS FOR ALL SYMBOLS WHERE CYCLES>0 (C) ==================================================")
    addCyclesForSymbols results.cAllSymbolCounters
    addLn ("")

    // addLn ("=========================================================== JVM FULL LISTINGS ===============================================================")
    // addLn ("")
    // sortedJvmMethodResults |> List.iter addJvmFullListing

    let result = (sb.ToString())
    result
