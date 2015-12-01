open System

type NmEntry = {
    name : string
    entryType : string
    address : int
    size : int
}

let nmlineToData (line : string) =
    let elements = line.Split()
    {
        name = elements.[3]
        entryType = elements.[2]
        address = Convert.ToInt32(elements.[0].Trim(), 16)
        size = Convert.ToInt32(elements.[1].Trim(), 16)
    }

let categories = [
    ("Lib/app/compiled code",
        [   ((=) "di_lib_infusions_archive_data")
            ((=) "di_app_infusion_archive_data")
            ((=) "rtc_compiled_code_buffer") ])
    ("Native C benchmark",
        [   ((=) "rtcbenchmark_measure_native_performance") ])
    ("Flash reprogramming",
        [   (fun name -> name.StartsWith("wkreprog_")) ])
    ("Darjeeling interpreter",
        [   ((=) "dj_exec_run")
            ((=) "BASTORE"); ((=) "BALOAD"); ((=) "CASTORE"); ((=) "CALOAD"); ((=) "SASTORE"); ((=) "SALOAD"); ((=) "IASTORE"); ((=) "IALOAD"); ((=) "LASTORE"); ((=) "LALOAD"); ((=) "AASTORE"); ((=) "AALOAD"); 
            (fun name -> name.StartsWith("GET"))
            (fun name -> name.StartsWith("PUT")) ])
    ("AOT compiler",
        [   (fun name -> name.StartsWith("rtc_") || name.StartsWith("RTC_") || name.StartsWith("emit") || name.StartsWith("asm_")) ])
    ]

let nameToCategory name =
    match categories |> List.tryFind (fun (cat, fs) -> fs |> List.exists (fun f -> f name)) with
    | Some (cat, fs) -> cat
    | None -> "Others" 

let totalSize entries = entries |> Seq.map (fun n -> n.size) |> Seq.sum
let categoryEntriesToString details (category, entries) =
    let entryToString entry =
        String.Format ("\t{0,10} {1} {2}", entry.size, entry.entryType, entry.name)
    String.Format("{0,10}: {1}{2}",
                  (totalSize entries),
                  category,
                  if details then "\r\n" + String.Join("\r\n", (entries |> Seq.map entryToString)) else "")

let filename = fsi.CommandLineArgs.[1]
let nmData = System.IO.File.ReadLines(filename) |> Seq.map nmlineToData
let categoryData =
    nmData
    |> Seq.filter (fun n -> (n.entryType <> "B"))
    |> Seq.groupBy (fun n -> (nameToCategory n.name))
    |> Seq.sortBy (fun (c, _) -> match categories |> List.tryFindIndex (fun (c2, fs) -> c = c2) with
                                 | Some(x) -> x
                                 | None -> -1)
    |> Seq.toList
Console.WriteLine(String.Join("\r\n\r\n", categoryData |> List.map (categoryEntriesToString true)))
Console.WriteLine("\r\n\r\n\r\n")
Console.WriteLine(String.Join("\r\n", categoryData |> List.map (categoryEntriesToString false)))
Console.WriteLine(String.Format("\r\n\r\nTOTAL SPACE ACCOUNTED FOR: {0}",
                                categoryData |> List.sumBy (fun (_, entries) -> (totalSize entries))))