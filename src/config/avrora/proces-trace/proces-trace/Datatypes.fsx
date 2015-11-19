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
    executedCyclesAOT : int;
    stopwatchCyclesC : int;
    stopwatchCyclesAOT : int;
    stopwatchCyclesJava : int;
    cyclesPush : ExecCounters;
    cyclesPop : ExecCounters;
    cyclesMovw : ExecCounters;
    cyclesPerJvmOpcode : list<string * ExecCounters>;
    cyclesPerJvmOpcodeCategory : list<string * ExecCounters>;
}