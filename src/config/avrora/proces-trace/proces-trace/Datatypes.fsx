type AvrInstruction = {
    address : int;
    opcode : int;
    text : string; }

type JvmInstruction = {
    index : int
    text : string }

type ExecCounters = {
    executions : int;
    cycles : int; }
    with
    static member (+) (x, y) = { executions = x.executions + y.executions ; cycles = x.cycles + y.cycles }
    member x.average = if x.executions > 0
                       then float x.cycles / float x.executions
                       else 0.0
    static member empty = { executions = 0; cycles = 0 }

type ResultAvr = {
    unopt : AvrInstruction;
    opt : AvrInstruction option;
    counters : ExecCounters }

type ResultJava = {
    jvm : JvmInstruction;
    avr : ResultAvr list;
    counters : ExecCounters }

type Results = {
    benchmark : string;
    jvmInstructions : ResultJava list;
    nativeCInstructions : (AvrInstruction*ExecCounters) list
    executedCyclesAOT : int;
    executedCyclesC : int;
    stopwatchCyclesJava : int;
    stopwatchCyclesAOT : int;
    stopwatchCyclesC : int;
    cyclesPush : ExecCounters;
    cyclesPop : ExecCounters;
    cyclesMovw : ExecCounters;
    cyclesPerJvmOpcode : (string * string * ExecCounters) list;
    cyclesPerAvrOpcodeAOTJava : (string * string * ExecCounters) list;
    cyclesPerAvrOpcodeNativeC : (string * string * ExecCounters) list;
    cyclesPerJvmOpcodeCategory : (string * ExecCounters) list;
    cyclesPerAvrOpcodeCategoryAOTJava : (string * ExecCounters) list
    cyclesPerAvrOpcodeCategoryNativeC : (string * ExecCounters) list;
}