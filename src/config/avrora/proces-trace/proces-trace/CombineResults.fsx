#load "Datatypes.fsx"

open System
open System.IO
open System.Linq
open Datatypes

let getLDDSTBytesJVM (result : SimulationResults) =
    0
let getLDDSTBytesFromAVRPerCategoryAOT (result : SimulationResults) =
    let isAVRloadstore (cat : string) =
        cat = "02) LD/ST rel to Y" || cat = "03) LD/ST rel to Z"
    let avrPerCategory = result.countersPerAvrOpcodeCategoryAOTJava
    let numberOfCycles = avrPerCategory |> List.filter (fun (cat, _) -> (isAVRloadstore cat))
                                       |> List.map (fun (cat, cnt) -> cnt.executions)
                                       |> List.fold (+) 0
    numberOfCycles / 2

let getLDDSTBytesFromAVRPerCategoryC (result : SimulationResults) =
    let isAVRloadstore (cat : string) =
        cat.Contains("LD/ST rel to")
    let avrPerCategory = result.countersPerAvrOpcodeCategoryNativeC
    let numberOfCycles = avrPerCategory |> List.filter (fun (cat, _) -> (isAVRloadstore cat))
                                       |> List.map (fun (cat, cnt) -> cnt.executions)
                                       |> List.fold (+) 0
    numberOfCycles / 2

let resultToStringList (result : SimulationResults) =
    let toPercentage totalCycles cycles =
        String.Format ("{0,5:0.0}", 100.0 * float cycles / float totalCycles)
    let cyclesToCPercentage = toPercentage result.countersCTotal.cycles
    let bytesToCPercentage = toPercentage result.countersCTotal.size
    let executedInstructionsJVM = result.countersPerJvmOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> cnt.executions) |> List.reduce (+)
    let executionsToPercentage = toPercentage executedInstructionsJVM
    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.00}", float cycles1 / float cycles2)
    let cyclesToOverhead1 cycles1 cycles2 =
        String.Format ("{0:0}%", float (cycles1-cycles2) / float cycles2 * 100.0)
    let cyclesToOverhead2 cycles1 cycles2 =
        String.Format ("{0:0}%", float (cycles1-cycles2) / float cycles1 * 100.0)

    let r1 =
        [
        ("BENCHMARK x"          , result.benchmark);
        ("Test"                 , if result.passedTestAOT then "PASSED" else "FAILED");
        (""                     , "");        
        ("PERF OVERHEAD"        , "cyc (%C)");
        ("   total"             , (cyclesToCPercentage result.countersOverheadTotal.cycles));
        ("   load/store"        , (cyclesToCPercentage result.countersOverheadLoadStore.cycles));
        ("   push/pop"          , (cyclesToCPercentage result.countersOverheadPushPop.cycles));
        ("   mov(w)"            , (cyclesToCPercentage result.countersOverheadMov.cycles));
        ("   vm+other"          , (cyclesToCPercentage (result.countersOverheadVm.cycles + result.countersOverheadOthers.cycles)));
        (""                     , "");
        ("   vm"                , (cyclesToCPercentage result.countersOverheadVm.cycles));
        ("   other"             , (cyclesToCPercentage result.countersOverheadOthers.cycles));
        (""                     , "");
        ("   stopwatch"         , (cyclesToCPercentage (result.cyclesStopwatchAOT - result.cyclesStopwatchC)));
        (""                     , "");
        ("SIZE OVERHEAD"        , "byt (%C)");
        ("   total"             , (bytesToCPercentage result.countersOverheadTotal.size));
        ("   load/store"        , (bytesToCPercentage result.countersOverheadLoadStore.size));
        ("   push/pop"          , (bytesToCPercentage result.countersOverheadPushPop.size));
        ("   mov(w)"            , (bytesToCPercentage result.countersOverheadMov.size));
        ("   other"             , (bytesToCPercentage result.countersOverheadOthers.size));
        (""                     , "");
        ("STOPWATCHES"          , "");
        ("Native C"             , result.cyclesStopwatchC.ToString());
        ("AOT"                  , result.cyclesStopwatchAOT.ToString());
        ("AOT/C"                , (cyclesToSlowdown result.cyclesStopwatchAOT result.cyclesStopwatchC));
        ("AOT overhead (%C)"    , (cyclesToOverhead1 result.cyclesStopwatchAOT result.cyclesStopwatchC));
        ("AOT overhead (%AOT)"  , (cyclesToOverhead2 result.cyclesStopwatchAOT result.cyclesStopwatchC));
        (""                     , "");
        ("CYCLE COUNTS"         , "");
        ("Native C total"       , String.Format("{0}", result.countersCTotal.cycles));
        ("         load/store"  , String.Format("{0}", result.countersCLoadStore.cycles));
        ("         push/pop"    , String.Format("{0}", result.countersCPushPop.cycles));
        ("         mov(w)"      , String.Format("{0}", result.countersCMov.cycles));
        ("         others"      , String.Format("{0}", result.countersCOthers.cycles));
        (""                     , "");
        ("         stopw/count" , (cyclesToSlowdown result.cyclesStopwatchC result.countersCTotal.cycles));
        (""                     , "");
        ("AOT      total"       , String.Format("{0}", result.countersAOTTotal.cycles));
        ("         load/store"  , String.Format("{0}", result.countersAOTLoadStore.cycles));
        ("         push/pop"    , String.Format("{0}", result.countersAOTPushPop.cycles));
        ("         mov(w)"      , String.Format("{0}", result.countersAOTMov.cycles));
        ("         vm"          , String.Format("{0}", result.countersAOTVM.cycles));
        ("         others"      , String.Format("{0}", result.countersAOTOthers.cycles));
        (""                     , "");
        ("         stopw/count" , (cyclesToSlowdown result.cyclesStopwatchAOT result.countersAOTTotal.cycles));
        (""                     , "");
        ("STOPW/(COUNT+TIMER)"  , "");
        ("Native C"             , (cyclesToSlowdown result.cyclesStopwatchC result.countersCTotalPlusTimer.cycles));
        ("AOT"                  , (cyclesToSlowdown result.cyclesStopwatchAOT result.countersAOTTotalPlusTimer.cycles));
        (""                     , "");
        ("MEMORY TRAFFIC"       , "");
        ("Bytes LD/ST AOT"      , String.Format ("{0}", (getLDDSTBytesFromAVRPerCategoryAOT result)));
        ("Bytes LD/ST C"        , String.Format ("{0}", (getLDDSTBytesFromAVRPerCategoryC result)));
        (""                     , "");
        ("STACK"                , "");
        ("max"                  , result.maxJvmStackInBytes.ToString());
        ("avg/executed jvm"     , String.Format ("{0:000.00}", result.avgJvmStackInBytes));
        ("avg change/exec jvm"  , String.Format ("{0:000.00}", result.avgJvmStackChangeInBytes));
        (""                     , "");
        ("CODE SIZE"            , "");
        ("Native C"             , result.codesizeC.ToString());
        ("AOT"                  , result.codesizeAOT.ToString());
        ("Java (AOT)"           , result.codesizeJavaForAOT.ToString());
        ("   branch overhead"   , (result.codesizeJavaForAOT - result.codesizeJavaWithoutBranchOverhead).ToString());
        ("   markloop overhead" , result.codesizeJavaMarkloopTotalSize.ToString());
        ("   Java ex. overhead" , result.codesizeJavaWithoutBranchMarkloopOverhead.ToString());
        ("Java (interpreter)"   , result.codesizeJavaForInterpreter.ToString())
        ("AOT/C"                , (cyclesToSlowdown result.codesizeAOT result.codesizeC));
        ("AOT/Java"             , (cyclesToSlowdown result.codesizeAOT result.codesizeJavaForAOT));
        (""                     , "");
        ("JVM exec (not cyc)"   , "");
        (" Load/Store"          , toPercentage result.countersJVMTotal.executions result.countersJVMLoadStore.executions)
        (" Constant load"       , toPercentage result.countersJVMTotal.executions result.countersJVMConstantLoad.executions)
        (" Processing"          , toPercentage result.countersJVMTotal.executions result.countersJVMProcessing.executions)
        ("     math"            , toPercentage result.countersJVMTotal.executions result.countersJVMProcessingMath.executions)
        ("     bit shift"       , toPercentage result.countersJVMTotal.executions result.countersJVMProcessingBitShift.executions)
        ("     bit logic"       , toPercentage result.countersJVMTotal.executions result.countersJVMProcessingBitLogic.executions)
        (" Branches"            , toPercentage result.countersJVMTotal.executions result.countersJVMBranches.executions)
        (" Invoke"              , toPercentage result.countersJVMTotal.executions result.countersJVMInvoke.executions)
        (" Others"              , toPercentage result.countersJVMTotal.executions result.countersJVMOthers.executions)
        (" Total"               , toPercentage result.countersJVMTotal.executions result.countersJVMTotal.executions)
        (""                     , "");
        (" Number of invokes"   , result.countersJVMInvoke.executions.ToString())
        ]
    let r2 = 
        (""                     , "")
        :: ("PERF AOT per JVM"  , "exe")
        :: (result.countersPerJvmOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (executionsToPercentage cnt.executions))))
    let r3 = 
        (""                     , "")
        :: ("PERF AOT per JVM"  , "cyc (%C)")
        :: (result.countersPerJvmOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    let r4 = 
        (""                     , "")
        :: ("PERF AOT per AVR"  , "cyc (%C)")
        :: (result.countersPerAvrOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    let r5 = 
        (""                     , "")
        :: ("PERF Native C"     , "cyc (%C)")
        :: (result.countersPerAvrOpcodeCategoryNativeC |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))

    let r6 = 
        (""                     , "")
        :: ("SIZE AOT per JVM"  , "byt (%C)")
        :: (result.countersPerJvmOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (bytesToCPercentage cnt.size))))
    let r7 = 
        (""                     , "")
        :: ("SIZE AOT per AVR"  , "byt (%C)")
        :: (result.countersPerAvrOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (bytesToCPercentage cnt.size))))
    let r8 = 
        (""                     , "")
        :: ("SIZE Native C"     , "byt (%C)")
        :: (result.countersPerAvrOpcodeCategoryNativeC |> List.map (fun (cat, cnt) -> (cat, (bytesToCPercentage cnt.size))))

    List.concat [ r1; r2; r3; r4; r5; r6; r7; r8 ]

let flipTupleListsToStringList (benchmarks : (string * string) list list) =
    // Initialise the accumulator as a list of lists containing only the key names
    let acc = benchmarks.Head |> List.map (fun (name, value) -> [name])
    // Then for each bm, add the values to the key name lists.
    let rec addValues (acc : string list list) (benchmarks : (string * string) list list) =
        match benchmarks with
        | head :: tail
            -> let acc2 = List.map2 (fun (_, headElement) accElement -> headElement :: accElement) head acc
               addValues acc2 tail
        | []
            -> acc
    // Elements will be in reversed order now.
    let wrongOrder = (addValues acc benchmarks)
    wrongOrder |> List.map (fun x -> x |> List.rev)

let addAverages (benchmarks : string list list) =
    benchmarks |> List.map (fun row ->
        match row with
        | [] -> []
        | "BENCHMARK" :: tail
            -> List.append row [""; "average"]
        | _ :: tail
            -> let numericValues = tail |> List.map (fun value ->
                                                        let success, res = Double.TryParse (value.Replace("%",""))
                                                        match success with
                                                        | true -> Some res
                                                        | false -> None)
                                        |> List.choose id
               let average = match numericValues with
                             | [] -> ""
                             | _  -> ((numericValues |> List.average).ToString("F1"))
               List.append row [""; average]
        )

let stringListToString (list : string list) =
    match list with
    | head :: tail
        -> String.Format ("{0,-20} {1}",
                            head,
                            String.Join(" ", tail |> List.map (fun x -> String.Format("{0,10}", x))))
    | [] -> ""





let summariseResults resultsDirectory =
    let resultFiles = Directory.GetFiles(resultsDirectory, "*.xml") |> Array.toList
    let resultsXmlStrings = resultFiles |> List.map (fun filename ->  printfn "Reading %s" (filename)
                                                                      File.ReadAllText(filename))
    let results =
        resultsXmlStrings
            |> List.map (fun xml -> (SimulationResults.unPickleOfString xml))
            |> List.sortBy (fun r -> let sortorder = ["bsort16"; "bsort32"; "hsort16"; "hsort32"; "binsrch16"; "binsrch32"; "fft"; "xxtea"; "md5"; "rc5"; "sortX"; "hsortX"; "binsrchX"; "coremk"; "coremk_cht"] in
                                     match sortorder |> List.tryFindIndex ((=) r.benchmark) with
                                     | Some (index) -> index
                                     | None -> 100)
    let resultsSummaryAsTupleLists = results |> List.map resultToStringList
    let resultsSummary = resultsSummaryAsTupleLists |> flipTupleListsToStringList
    let resultsSummaryWithAvg = resultsSummary |> addAverages
    let resultLines = resultsSummaryWithAvg |> List.map stringListToString

    let summaryFilename = if (resultsDirectory.StartsWith("/"))
                          then resultsDirectory + "/summary" + (Path.GetFileName(resultsDirectory).Replace("results",""))
                          else resultsDirectory + "/summary" + (resultsDirectory.Replace("results",""))
    File.WriteAllText (summaryFilename, String.Join("\r\n", resultLines))
    Console.Error.WriteLine ("Wrote output to " + summaryFilename)

let main(args : string[]) =
    Console.Error.WriteLine ("START " + (DateTime.Now.ToString()))
    let arg = (Array.get args 1)
    match arg with
    | "all" -> 
        let directory = (Array.get args 2)
        let subdirectories = (Directory.GetDirectories(directory))
        subdirectories |> Array.filter (fun d -> ((Path.GetFileName(d).StartsWith("results_")) && not (Path.GetFileName(d).StartsWith("results_coremk_c"))))
                       |> Array.iter summariseResults
    | resultsbasedir -> summariseResults resultsbasedir
    Console.Error.WriteLine ("STOP " + (DateTime.Now.ToString()))
    1

main(fsi.CommandLineArgs)
