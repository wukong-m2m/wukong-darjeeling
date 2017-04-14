// tijdelijk omdat ik geen tijd heb alle simulaties opnieuw te doen.
// dus de code size data uit de eerder traces halen

#r "binaries/FSharp.Data/FSharp.Data.dll"
#r "binaries/fspickler.3.2.0/lib/net45/FsPickler.dll"
#load "Datatypes.fsx"
#load "AVR.fsx"

open System
open System.IO
open System.Linq
open System.Text.RegularExpressions
open System.Runtime.Serialization
open FSharp.Data
open MBrace.FsPickler
open Datatypes

let configurations = ["results_0BASE_R___P__CS0"; "results_0BASE_R___P__CS0_simple_opt"; "results_1SMPL_R11_P__CS0"; "results_2POPD_R11_P__CS0"; "results_3MARK_R11_P7_CS0"; "results_3MARK_R11_P7_CS4"]
let benchmarks = ["bsort32"; "hsort32"; "binsrch32"; "fft"; "xxtea"; "md5"; "rc5"]

let instructionCategoryC (avr : AvrInstruction) =
    let opcode = AVR.getOpcodeForInstruction avr.opcode avr.text in
    let cat = AVR.opcodeCategory opcode
    match cat with
    | "01) LD/ST rel to X" -> "load/store"
    | "02) LD/ST rel to Y" -> "load/store"
    | "03) LD/ST rel to Z" -> "load/store"
    | "04) Stack push/pop" -> "push/pop"
    | "05) Register moves" -> "mov"
    | "06) Constant load" -> "others"
    | "07) Comp./branches" -> "others"
    | "08) Math" -> "others"
    | "09) Bit shifts" -> "others"
    | "10) Bit logic" -> "others"
    | "11) Subroutines" -> "others"
    | "12) Others" -> "others"
    | _ -> failwith "?"

// For avr ld/st to X are actually pushes
let instructionCategoryAOT (avr : AvrInstruction) =
    let opcode = AVR.getOpcodeForInstruction avr.opcode avr.text in
    let cat = AVR.opcodeCategory opcode
    match cat with
    | "01) LD/ST rel to X" -> "push/pop"
    | "02) LD/ST rel to Y" -> "load/store"
    | "03) LD/ST rel to Z" -> "load/store"
    | "04) Stack push/pop" -> "push/pop"
    | "05) Register moves" -> "mov"
    | "06) Constant load" -> "others"
    | "07) Comp./branches" -> "others"
    | "08) Math" -> "others"
    | "09) Bit shifts" -> "others"
    | "10) Bit logic" -> "others"
    | "11) Subroutines" -> "others"
    | "12) Others" -> "others"
    | _ -> failwith "?"

let groupFold keyFunc valueFunc foldFunc foldInitAcc x =
    x |> List.toSeq
      |> Seq.groupBy keyFunc
      |> Seq.map (fun (key, groupedResults) -> (key, groupedResults |> Seq.map valueFunc |> Seq.fold foldFunc foldInitAcc))
      |> Seq.toList

type CodesizeResult = {
    configuration : string;
    benchmark : string;    
    pushpopC : int;
    loadstoreC : int;
    movC : int;
    othersC : int;
    pushpopAOT : int;
    loadstoreAOT : int;
    movAOT : int;
    othersAOT : int; }
    with
    member this.totalC = this.pushpopC + this.loadstoreC + this.movC + this.othersC;
    member this.totalAOT = this.pushpopAOT + this.loadstoreAOT + this.movAOT + this.othersAOT;

let getResults configuration benchmark =
    let filename = String.Format("/Users/niels/CASES2016-results/spmc_results/{0}/{1}.xml", configuration, benchmark)
    let xml = FsPickler.CreateXmlSerializer(indent = true)
    let results = xml.UnPickleOfString<Results>(File.ReadAllText(filename))
    let instructionsC = results.nativeCInstructions
                        |> List.map (fun (avr, _) -> (instructionCategoryC avr, (if avr.opcode<=0xFFFF then 2 else 4)))
    let groupedInstructionsC = groupFold fst snd (+) 0 instructionsC

    let instructionsAOT = results.jvmInstructions
                          |> List.collect (fun jvm -> jvm.avr)
                          |> List.map (fun avr -> avr.opt)
                          |> List.choose id
                          |> List.map (fun avr -> (instructionCategoryAOT avr, (if avr.opcode<=0xFFFF then 2 else 4)))
    let groupedInstructionsAOT = groupFold fst snd (+) 0 instructionsAOT

    let tryFindCategory groupedInstructions cat =
        match groupedInstructions |> List.tryFind (fun (c, _) -> c = cat) with
        | Some (_, x) -> x
        | None -> 0
    {
        configuration = configuration
        benchmark     = benchmark
        pushpopC      = tryFindCategory groupedInstructionsC "push/pop"
        loadstoreC    = tryFindCategory groupedInstructionsC "load/store"
        movC          = tryFindCategory groupedInstructionsC "mov"
        othersC       = tryFindCategory groupedInstructionsC "others"
        pushpopAOT    = tryFindCategory groupedInstructionsAOT "push/pop"
        loadstoreAOT  = tryFindCategory groupedInstructionsAOT "load/store"
        movAOT        = tryFindCategory groupedInstructionsAOT "mov"
        othersAOT     = tryFindCategory groupedInstructionsAOT "others"
    }

let alles = configurations |> List.map (fun config ->
                (config, (benchmarks |> List.map (fun benchmark -> (getResults config benchmark)))))

let alleStrings = alles |> List.collect (fun (config, bmresults) ->
                           bmresults |> List.collect (fun bmresult -> [
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "pushpop", "C", bmresult.pushpopC);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "loadstore", "C", bmresult.loadstoreC);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "mov", "C", bmresult.movC);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "others", "C", bmresult.othersC);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "total", "C", bmresult.totalC);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "pushpop", "AOT", bmresult.pushpopAOT);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "loadstore", "AOT", bmresult.loadstoreAOT);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "mov", "AOT", bmresult.movAOT);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "others", "AOT", bmresult.othersAOT);
                                                                        String.Format("{0,-30} {1,-10} {2,-10} {3,3} {4,10}", config, bmresult.benchmark, "total", "AOT", bmresult.totalAOT)
                                                 ]))

let x = alleStrings
        |> List.fold (fun acc y -> acc + "\r\n" + y) ""
Console.WriteLine(x);

// Console.WriteLine ("AOT")
// let x = groupedInstructionsAOT
//         |> List.fold (fun acc (cat, totalsize) -> acc + "\r\n" + String.Format("{0} {1}", cat, totalsize)) ""
// Console.WriteLine (x)
// Console.WriteLine ("C")
// let y = groupedInstructionsC
//         |> List.fold (fun acc (cat, totalsize) -> acc + "\r\n" + String.Format("{0} {1}", cat, totalsize)) ""

//let y = results.nativeCInstructions
//        |> List.map (fun (avr, _) -> (instructionCategoryC avr, (if avr.opcode<=0xFFFF then 1 else 2 ), avr.text))
//        |> List.fold (fun acc (cat, size, text) -> acc + "\r\n" + String.Format("{0} {1} {2}", cat, size, text)) ""
//Console.WriteLine (y)
