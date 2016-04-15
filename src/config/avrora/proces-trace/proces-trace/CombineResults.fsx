#r "binaries/fspickler.1.5.2/lib/net45/FsPickler.dll"
#load "Datatypes.fsx"

open System
open System.IO
open System.Linq
open Nessos.FsPickler
open Datatypes




let getLDDSTBytesJVM (result : Results) =
    0
let getLDDSTBytesFromAVRPerCategoryAOT (result : Results) =
    let isAVRloadstore (cat : string) =
        cat = "02) LD/ST rel to Y" || cat = "03) LD/ST rel to Z"
    let avrPerCategory = result.cyclesPerAvrOpcodeCategoryAOTJava
    let numberOfCycles = avrPerCategory |> List.filter (fun (cat, _) -> (isAVRloadstore cat))
                                       |> List.map (fun (cat, cnt) -> cnt.executions)
                                       |> List.fold (+) 0
    numberOfCycles / 2

let getLDDSTBytesFromAVRPerCategoryC (result : Results) =
    let isAVRloadstore (cat : string) =
        cat.Contains("LD/ST rel to")
    let avrPerCategory = result.cyclesPerAvrOpcodeCategoryNativeC
    let numberOfCycles = avrPerCategory |> List.filter (fun (cat, _) -> (isAVRloadstore cat))
                                       |> List.map (fun (cat, cnt) -> cnt.executions)
                                       |> List.fold (+) 0
    numberOfCycles / 2

let resultToStringList (result : Results) =
    let cyclesToPercentage totalCycles cycles =
        String.Format ("{0,5:0.0}", 100.0 * float cycles / float totalCycles)
    let cyclesToAOTPercentage = cyclesToPercentage result.executedCyclesAOT
    let cyclesToCPercentage = cyclesToPercentage result.executedCyclesC
    let executedInstructionsJVM = result.cyclesPerJvmOpcodeCategory |> List.map (fun (cat, cnt) -> cnt.executions) |> List.reduce (+)
    let executionsToPercentage = cyclesToPercentage executedInstructionsJVM
    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.00}", float cycles1 / float cycles2)
    let cyclesToOverhead1 cycles1 cycles2 =
        String.Format ("{0:0}%", float (cycles1-cycles2) / float cycles2 * 100.0)
    let cyclesToOverhead2 cycles1 cycles2 =
        String.Format ("{0:0}%", float (cycles1-cycles2) / float cycles1 * 100.0)


    // let overheadInCycles = result.stopwatchCyclesAOT-result.stopwatchCyclesC
    // let overheadLoadStoreInCycles = ((getLDDSTBytesFromAVRPerCategoryAOT result)-(getLDDSTBytesFromAVRPerCategoryC result))*2
    // let overheadPushPopInCycles =
    //     let nativeCPushPopInCycles = (result.cyclesPerAvrOpcodeCategoryNativeC |> List.find (fun (cat, cnt) -> cat.StartsWith("04) Stack")) |> snd).cycles
    //     result.cyclesPush.cycles + result.cyclesPop.cycles - nativeCPushPopInCycles
    // let overheadMovwInCycles =
    //     let nativeCMovInCycles = (result.cyclesPerAvrOpcodeCategoryNativeC |> List.find (fun (cat, cnt) -> cat.StartsWith("05) Register moves")) |> snd).cycles
    //     result.cyclesMovw.cycles - nativeCMovInCycles

    let r1 =
        [
        ("BENCHMARK"            , result.benchmark);
        ("Test"                 , if result.passedTestAOT then "PASSED" else "FAILED");
        (""                     , "");        
        ("STOPWATCHES"          , "");
        ("Native C"             , result.stopwatchCyclesC.ToString());
        ("AOT"                  , result.stopwatchCyclesAOT.ToString());
        ("Java"                 , result.stopwatchCyclesJava.ToString());
        ("AOT/C"                , (cyclesToSlowdown result.stopwatchCyclesAOT result.stopwatchCyclesC));
        ("AOT overhead (%C)"    , (cyclesToOverhead1 result.stopwatchCyclesAOT result.stopwatchCyclesC));
        ("AOT overhead (%AOT)"  , (cyclesToOverhead2 result.stopwatchCyclesAOT result.stopwatchCyclesC));
        ("Java/C"               , (cyclesToSlowdown result.stopwatchCyclesJava result.stopwatchCyclesC));
        ("Java/AOT"             , (cyclesToSlowdown result.stopwatchCyclesJava result.stopwatchCyclesAOT));
        (""                     , "");
        ("CYCLE COUNTS"         , "");
        ("Native C total"       , String.Format("{0}", result.cyclesCTotal));
        ("         push/pop"    , String.Format("{0}", result.cyclesCPushPop.cycles));
        ("         mov(w)"      , String.Format("{0}", result.cyclesCMov.cycles));
        ("         load/store"  , String.Format("{0}", result.cyclesCLoadStore.cycles));
        ("AOT      total"       , String.Format("{0}", result.cyclesAOTTotal));
        ("         stopw/count" , (cyclesToSlowdown result.stopwatchCyclesAOT result.cyclesAOTTotal));
        ("         push/pop"    , String.Format("{0}", result.cyclesAOTPushPopInt.cycles+result.cyclesAOTPushPopRef.cycles));
        ("         mov(w)"      , String.Format("{0}", result.cyclesAOTMov.cycles));
        ("         load/store"  , String.Format("{0}", result.cyclesAOTLoadStore.cycles));
        (""                     , "");
        ("OVERHEAD (%C)"        , "");
        ("   push/pop"          , (cyclesToPercentage result.cyclesCTotal result.overheadPushPop.cycles));
        ("   mov(w)"            , (cyclesToPercentage result.cyclesCTotal result.overheadMov.cycles));
        ("   load/store"        , (cyclesToPercentage result.cyclesCTotal result.overheadLoadStore.cycles));
        ("   other"             , (cyclesToPercentage result.cyclesCTotal (result.overheadTotalCycles - result.overheadPushPop.cycles - result.overheadMov.cycles - result.overheadLoadStore.cycles)));
        ("   total"             , (cyclesToPercentage result.cyclesCTotal result.overheadTotalCycles));
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
        ("Java"                 , result.codesizeJava.ToString());
        ("  branch overhead"    , (result.codesizeJava - result.codesizeJavaWithoutBranchOverhead).ToString());
        ("  markloop overhead"  , result.codesizeJavaMarkloopTotalSize.ToString());
        ("  Java ex. overhead"  , result.codesizeJavaWithoutBranchMarkloopOverhead.ToString());
        ("AOT/C"                , (cyclesToSlowdown result.codesizeAOT result.codesizeC));
        ("AOT/Java"             , (cyclesToSlowdown result.codesizeAOT result.codesizeJava));
        ]
    let r2 = 
        (""                     , "")
        :: ("JVM EXE (not cyc!)", "")
        :: (result.cyclesPerJvmOpcodeCategory |> List.map (fun (cat, cnt) -> (cat, (executionsToPercentage cnt.executions))))
    let r3 = 
        (""                     , "")
        :: ("JVM (%C)"          , "")
        :: (result.cyclesPerJvmOpcodeCategory |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    let r4 = 
        (""                     , "")
        :: ("AVR Java AOT (%C)" , "")
        :: (result.cyclesPerAvrOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    let r5 = 
        (""                     , "")
        :: ("AVR Native C"      , "")
        :: (result.cyclesPerAvrOpcodeCategoryNativeC |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    List.concat [ r1; r2; r3; r4; r5 ]

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

let stringListToString (list : string list) =
    match list with
    | head :: tail
        -> String.Format ("{0,-20} {1}",
                            head,
                            String.Join(" ", tail |> List.map (fun x -> String.Format("{0,10}", x))))
    | [] -> ""

let summariseResults resultsDirectory =
    let xmlSerializer = FsPickler.CreateXmlSerializer(indent = true)

    let resultFiles = Directory.GetFiles(resultsDirectory, "*.xml") |> Array.toList
    let resultsXmlStrings = resultFiles |> List.map (fun filename -> File.ReadAllText(filename))
    let results =
        resultsXmlStrings
            |> List.map (fun xml -> xmlSerializer.UnPickleOfString<Results> xml)
            |> List.sortBy (fun r -> let sortorder = ["bsort16"; "bsort32"; "hsort16"; "hsort32"; "binsrch16"; "binsrch32"; "fft"; "xxtea"; "md5"; "rc5"; "sortX"; "hsortX"; "binsrchX"] in
                                     match sortorder |> List.tryFindIndex ((=) r.benchmark) with
                                     | Some (index) -> index
                                     | None -> 100)
    let resultsSummaryAsTupleLists = results |> List.map resultToStringList
    let resultsSummary = resultsSummaryAsTupleLists |> flipTupleListsToStringList
    let resultLines = resultsSummary |> List.map stringListToString

    let csvFilename = resultsDirectory + "/summary" + (resultsDirectory.Replace("./results","").Replace("results","")) + ".csv"
    File.WriteAllText (csvFilename, String.Join("\r\n", resultLines))
    Console.Error.WriteLine ("Wrote output to " + csvFilename)

let main(args : string[]) =
    if (args.Count() >= 2)
    then
        summariseResults (Array.get args 1)
        1
    else
        0

main(fsi.CommandLineArgs)
//summariseResults "/Users/niels/src/rtc/src/config/avrora/results"
