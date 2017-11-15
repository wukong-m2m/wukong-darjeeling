open ProcessOutput.ProcessTraces 
open ProcessOutput.CombineResults

[<EntryPoint>]
let main argv = 
    match (Array.toList argv) with
    | head::tail -> match head with
                    | "process" -> (ProcessOutput.ProcessTraces.main tail)
                    | "combine" -> (ProcessOutput.CombineResults.main tail)
                    | other -> failwithf "Illegal option: %s. Should be either 'process' or 'combine'" other
    | _ -> failwith "Usage: process-output [process|combine] parameters"
