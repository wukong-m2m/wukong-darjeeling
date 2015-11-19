#r "binaries/fspickler.1.5.2/lib/net45/FsPickler.dll"
#load "Datatypes.fsx"

open System
open System.IO
open System.Linq
open Nessos.FsPickler
open Datatypes

let resultToStringList (result : Results) =
    let cyclesToPercentage cycles =
        String.Format ("{0:00.000}", 100.0 * float cycles / float result.executedCyclesAOT)
    let cyclesToSlowdown cycles1 cycles2 =
        String.Format ("{0:0.00}", float cycles1 / float cycles2)
    let r1 =
        [
        ("BENCHMARK"         , result.benchmark);
        (""                  , "");
        ("STOPWATCHES"       , "");
        ("Native C"          , result.stopwatchCyclesC.ToString());
        ("AOT"               , result.stopwatchCyclesAOT.ToString());
        ("Java"              , result.stopwatchCyclesJava.ToString());
        ("AOT/C"             , (cyclesToSlowdown result.stopwatchCyclesAOT result.stopwatchCyclesC));
        ("Java/C"            , (cyclesToSlowdown result.stopwatchCyclesJava result.stopwatchCyclesC));
        ("Java/AOT"          , (cyclesToSlowdown result.stopwatchCyclesJava result.stopwatchCyclesAOT));
        (""                  , "");
        ("CYCLE COUNTS"      , "");
        ("AOT method total"  , result.executedCyclesAOT.ToString());
        ("AOT stopw/count"   , (cyclesToSlowdown result.stopwatchCyclesAOT result.executedCyclesAOT));
        ("PUSH"              , (cyclesToPercentage result.cyclesPush.cycles));
        ("POP"               , (cyclesToPercentage result.cyclesPop.cycles));
        ("MOVW"              , (cyclesToPercentage result.cyclesMovw.cycles));
        ("PUSH+POP+MOVW"     , (cyclesToPercentage (result.cyclesPush.cycles+result.cyclesPop.cycles+result.cyclesMovw.cycles)));
        (""                  , "");
        ("OPCODE CATEGORIES" , "")
        ]
    let r2 = result.cyclesPerJvmOpcodeCategory |> List.map (fun (cat, cnt) -> (cat, (cyclesToPercentage cnt.cycles)))
    List.concat [ r1 ; r2 ]

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
        -> String.Format ("{0,20},{1}",
            head,
            String.Join(",", tail |> List.map (fun x -> String.Format("{0,10}", x))))
    | [] -> ""

let summariseResults resultsDirectory =
    let xmlSerializer = FsPickler.CreateXmlSerializer(indent = true)

    let resultFiles = Directory.GetFiles(resultsDirectory, "*.xml") |> Array.toList
    let resultsXmlStrings = resultFiles |> List.map (fun filename -> File.ReadAllText(filename))
    let results = resultsXmlStrings |> List.map (fun xml -> xmlSerializer.UnPickleOfString<Results> xml)
    let resultsSummaryAsTupleLists = results |> List.map resultToStringList
    let resultsSummary = resultsSummaryAsTupleLists |> flipTupleListsToStringList
    let resultLines = resultsSummary |> List.map stringListToString

    let csvFilename = resultsDirectory + "/summary.csv"
    File.WriteAllText (csvFilename, String.Join("\r\n", resultLines))
    Console.Error.WriteLine ("Wrote output to " + csvFilename)

let main(args : string[]) =
    if (args.Count() >= 2)
    then
        summariseResults (Array.get args 1)
        1
    else
        0

//main(fsi.CommandLineArgs)
summariseResults "/Users/niels/src/rtc/src/config/avrora/results"
