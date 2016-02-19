#r "binaries/fspickler.1.5.2/lib/net45/FsPickler.dll"
#load "Datatypes.fsx"

open System
open System.IO
open System.Linq
open Nessos.FsPickler
open Datatypes

let resultToStringList (result : Results) =
    let cyclesToPercentage totalCycles cycles =
        String.Format ("{0,5:0.0}", 100.0 * float cycles / float totalCycles)
    let cyclesToAOTPercentage = cyclesToPercentage result.executedCyclesAOT
    let cyclesToCPercentage = cyclesToPercentage result.executedCyclesC
    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.00}", float cycles1 / float cycles2)
    let cyclesToOverhead1 cycles1 cycles2 =
        String.Format ("{0:0}%", float (cycles1-cycles2) / float cycles2 * 100.0)
    let cyclesToOverhead2 cycles1 cycles2 =
        String.Format ("{0:0}%", float (cycles1-cycles2) / float cycles1 * 100.0)
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
        ("AOT method total"     , result.executedCyclesAOT.ToString());
        ("AOT stopw/count"      , (cyclesToSlowdown result.stopwatchCyclesAOT result.executedCyclesAOT));
        ("PUSH"                 , (cyclesToAOTPercentage result.cyclesPush.cycles));
        ("POP"                  , (cyclesToAOTPercentage result.cyclesPop.cycles));
        ("MOVW"                 , (cyclesToAOTPercentage result.cyclesMovw.cycles));
        ("PUSH+POP+MOVW"        , (cyclesToAOTPercentage (result.cyclesPush.cycles+result.cyclesPop.cycles+result.cyclesMovw.cycles)));
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
        ("AOT/C"                , (cyclesToSlowdown result.codesizeAOT result.codesizeC));
        ("AOT/Java"             , (cyclesToSlowdown result.codesizeAOT result.codesizeJava));
        ]
    let r2 = 
        (""                     , "")
        :: ("JVM (%C)"          , "")
        :: (result.cyclesPerJvmOpcodeCategory |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    let r3 = 
        (""                     , "")
        :: ("AVR Java AOT (%C)" , "")
        :: (result.cyclesPerAvrOpcodeCategoryAOTJava |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    let r4 = 
        (""                     , "")
        :: ("AVR Native C"      , "")
        :: (result.cyclesPerAvrOpcodeCategoryNativeC |> List.map (fun (cat, cnt) -> (cat, (cyclesToCPercentage cnt.cycles))))
    List.concat [ r1; r2; r3; r4 ]

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
            |> List.sortBy (fun r -> let sortorder = ["bsort16"; "bsort32"; "hsort16"; "hsort32"; "binsrch16"; "binsrch32"; "fft"; "rc5"; "xxtea"; "md5"; "sortX"; "hsortX"; "binsrchX"] in
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
