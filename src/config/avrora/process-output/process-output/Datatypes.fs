namespace ProcessOutput

module Datatypes =
    open Helpers
    open FSharp.Data
    open System.Web
    open MBrace.FsPickler

    type RtcdataXml = XmlProvider<"examples/rtcdata-example.xml", Global=true>
    type Rtcdata = RtcdataXml.Methods
    type MethodImpl = RtcdataXml.MethodImpl
    type ProfilerdataXml = XmlProvider<"examples/profilerdata-example.xml", Global=true>
    type Profilerdata = ProfilerdataXml.ExecutionCountPerInstruction
    type ProfiledInstruction = ProfilerdataXml.Instruction

    let getClassAndMethodNameFromFullName (fullname : string) =
        let indexOfLastDot = fullname.LastIndexOf(".");
        let indexOfSecondLastDot = match indexOfLastDot with
                                   | -1 -> -1
                                   | 0 -> -1
                                   | _ -> fullname.LastIndexOf(".", indexOfLastDot-1)
        let methodName = match indexOfSecondLastDot with
                         | -1 -> fullname
                         | _ -> fullname.Substring(indexOfSecondLastDot+1)
        HttpUtility.UrlDecode(methodName)

    let getClassAndMethodNameFromImpl (impl : MethodImpl) =
        getClassAndMethodNameFromFullName impl.Method

    let getMethodNameFromImpl (impl : MethodImpl) =
        let classAndMethodName = getClassAndMethodNameFromImpl impl
        classAndMethodName.Split([|'.'|]).[1]

    type AvrInstruction = {
        address : int
        opcode : int
        text : string }

    type JvmInstruction = {
        index : int
        text : string }
        with
        member this.isInvoke = this.text.StartsWith("JVM_INVOKE")
        member this.instructionOnly =
            let firstSpaceIndex = this.text.IndexOf(" ")
            match firstSpaceIndex with
            | -1    -> this.text                               // No space: return whole text
            | index -> this.text.Substring(0, index)           // Strip whatever comes after the space from the instruction
        member this.instructionDetails =
            let firstSpaceIndex = this.text.IndexOf(" ")
            match firstSpaceIndex with
            | -1    -> ""                                      // No space: empty string
            | index -> this.text.Substring(firstSpaceIndex+1)  // Return whatever comes after the space (which method is called, which field is loaded/stored, etc.)

    type ExecCounters = {
        executions : int
        cycles : int
        cyclesSubroutine : int
        count : int
        size : int }
        with
        static member (+) (x, y) = { executions = x.executions + y.executions ; cycles = x.cycles + y.cycles ; cyclesSubroutine = x.cyclesSubroutine + y.cyclesSubroutine ; count = x.count + y.count ; size = x.size + y.size }
        static member (-) (x, y) = { executions = x.executions - y.executions ; cycles = x.cycles - y.cycles ; cyclesSubroutine = x.cyclesSubroutine - y.cyclesSubroutine ; count = x.count - y.count ; size = x.size - y.size }
        member this.cyclesInclSubroutine = this.cycles + this.cyclesSubroutine
        member this.averageInclSubroutine = if this.executions > 0
                                            then float this.cyclesInclSubroutine / float this.executions
                                            else 0.0
        member this.average = if this.executions > 0
                              then float this.cycles / float this.executions
                              else 0.0
        static member Zero  = { executions = 0; cycles = 0; cyclesSubroutine = 0; count = 0; size = 0 }

    type ResultAvr = {
        unopt : AvrInstruction;
        opt : AvrInstruction option;
        counters : ExecCounters }

    type StackDatatype = Ref | Boolean | Byte | Char | Short | Int
    let StackDatatypeToSize x =
        match x with // stack slot width is 16 bit, so everything takes at least two bytes.
        | Ref -> 2
        | Boolean -> 2
        | Byte -> 2
        | Char -> 2
        | Short -> 2
        | Int -> 4
    let StackDatatypeToString x =
        match x with
        | Ref -> "Ref"
        | Boolean -> "Boolean"
        | Byte -> "Byte"
        | Char -> "Char"
        | Short -> "Short"
        | Int -> "Int"
    let StackDatatypeFromString x =
        match x with
        | "Ref" -> Ref
        | "Boolean" -> Boolean
        | "Byte" -> Byte
        | "Char" -> Char
        | "Short" -> Short
        | "Int" -> Int
        | _ -> failwith ("Unknown datatype '" + x + "'")

    type StackElement = {
        origin : int
        datatype : StackDatatype
    }

    type DJDebugData = {
        byteOffset : int;
        instOffset : int;
        text : string;
        stackBefore : StackElement list;
        stackAfter : StackElement list; }
        with
        static member empty = { byteOffset = 0; instOffset = 0; text = ""; stackBefore = []; stackAfter = [] }
        member this.stackSizeBefore = this.stackBefore |> List.map (fun x -> x.datatype) |> List.sumBy StackDatatypeToSize
        member this.stackSizeAfter = this.stackAfter |> List.map (fun x -> x.datatype) |> List.sumBy StackDatatypeToSize

    type ProcessedJvmInstruction = {
        jvm : JvmInstruction;
        avr : ResultAvr list;
        counters : ExecCounters;
        djDebugData : DJDebugData; }

    let sumCountersPerCategory (countersPerAvrCategory : (string * ExecCounters) list) =
        countersPerAvrCategory |> groupFold fst snd (+) ExecCounters.Zero

    let sumCountersPerOpcodeAndCategory (countersPerAvrOpcodeAndCategory : (string * string * ExecCounters) list) =
        countersPerAvrOpcodeAndCategory |> List.map (fun (opcode,cat,cnt) -> ((opcode,cat),cnt))
                                        |> groupFold fst snd (+) ExecCounters.Zero
                                        |> List.map (fun ((opcode,cat),cnt) -> (opcode,cat,cnt))

    let sumCyclesCategoryCounters (cyclesPerAvrOpcodeCategory) (catTest : string -> bool) =
        cyclesPerAvrOpcodeCategory |> List.filter (fun (cat,cnt) -> (catTest cat))
                                   |> List.map snd
                                   |> List.fold (+) ExecCounters.Zero

    let groupOpcodesInCategoriesCycles (allCategories : string list) (results : (string * string * ExecCounters) list) =
        let categoriesPresent =
            results
                |> List.map (fun (cat, opcode, cnt) -> (cat, cnt))
                |> groupFold fst snd (+) ExecCounters.Zero
        allCategories
            |> List.sort
            |> List.map (fun cat -> match categoriesPresent |> List.tryFind (fun (cat2, _) -> cat = cat2) with
                                    | Some (cat, cnt) -> (cat, cnt)
                                    | None -> (cat, ExecCounters.Zero))

    let groupOpcodesInCategoriesBytes (allCategories : string list) (results : (string * string * int) list) =
        let categoriesPresent =
            results
                |> List.map (fun (cat, opcode, cnt) -> (cat, cnt))
                |> groupFold fst snd (+) 0
        allCategories
            |> List.sort
            |> List.map (fun cat -> match categoriesPresent |> List.tryFind (fun (cat2, _) -> cat = cat2) with
                                    | Some (cat, cnt) -> (cat, cnt)
                                    | None -> (cat, 0))


    type IgnoreFSI_NamespaceConverter (?ignoreVersion : bool) =
        let ignoreVersion = defaultArg ignoreVersion true
        interface ITypeNameConverter with
            member __.OfSerializedType (tI : TypeInfo) =
                { tI with Name = "SimulationResults" }
            member __.ToDeserializedType (tI : TypeInfo) =
                { tI with Name = "SimulationResults" }

    type JvmMethod = {
        name : string;
        instructions : ProcessedJvmInstruction list;

        codesizeJavaForAOT : int;
        codesizeJavaBranchCount : int;
        codesizeJavaBranchTargetCount : int;
        codesizeJavaMarkloopCount : int;
        codesizeJavaMarkloopTotalSize : int;
        codesizeJavaWithoutBranchOverhead : int;
        codesizeJavaWithoutBranchMarkloopOverhead : int;
        codesizeJavaForInterpreter : int;
        codesizeAOT : int;

        countersPerJvmOpcodeAOTJava : (string * string * ExecCounters) list;
        countersPerAvrOpcodeAOTJava : (string * string * ExecCounters) list;
        }
        with
        member this.countersPerJvmOpcodeCategoryAOTJava = groupOpcodesInCategoriesCycles JVM.getAllJvmOpcodeCategories this.countersPerJvmOpcodeAOTJava
        member this.countersPerAvrOpcodeCategoryAOTJava = groupOpcodesInCategoriesCycles AVR.getAllOpcodeCategories    this.countersPerAvrOpcodeAOTJava


        member this.countersJVMLoadStore                = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Loc/sta")
                                                                                                                                           || cat.Contains("Obj/arr"))
        member this.countersJVMConstantLoad             = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Constant load"))
        member this.countersJVMProcessingMath           = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Math"))
        member this.countersJVMProcessingBitShift       = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Bit shift"))
        member this.countersJVMProcessingBitLogic       = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Bit logic"))
        member this.countersJVMProcessing               = this.countersJVMProcessingMath + this.countersJVMProcessingBitShift + this.countersJVMProcessingBitLogic
        member this.countersJVMBranches                 = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Branches"))
        member this.countersJVMInvoke                   = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Invoke"))
        member this.countersJVMOthers                   = sumCyclesCategoryCounters this.countersPerJvmOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Conversions")
                                                                                                                                           || cat.Contains("Markloop")
                                                                                                                                           || cat.Contains("Others")
                                                                                                                                           || cat.Contains("VM"))
        member this.countersJVMTotal                    = this.countersPerJvmOpcodeCategoryAOTJava |> List.sumBy (fun (_, cnt) -> cnt)


        member this.countersAOTLoadStore                = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("LD/ST rel to") && not (cat.Contains("LD/ST rel to X")))
        member this.countersAOTPushPopInt               = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Stack push/pop"))
        member this.countersAOTPushPopRef               = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("LD/ST rel to X"))
        member this.countersAOTPushPop                  = this.countersAOTPushPopInt + this.countersAOTPushPopRef
        member this.countersAOTMov                      = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> cat.Contains("Register moves"))
        member this.countersAOTOthers                   = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryAOTJava (fun (cat) -> not (cat.Contains("LD/ST rel to")) && not (cat.Contains("Stack push/pop")) && not (cat.Contains("Register moves")))
        member this.countersAOTTotal                    = this.countersAOTPushPop + this.countersAOTMov + this.countersAOTLoadStore + this.countersAOTOthers;

    type CFunction = {
        name : string;
        instructions : (AvrInstruction*ExecCounters) list;

        codesizeC : int;

        countersPerAvrOpcodeNativeC : (string * string * ExecCounters) list;
        }
        with
        member this.countersPerAvrOpcodeCategoryNativeC = groupOpcodesInCategoriesCycles AVR.getAllOpcodeCategories    this.countersPerAvrOpcodeNativeC

        member this.countersCLoadStore                  = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> cat.Contains("LD/ST rel to"))
        member this.countersCPushPop                    = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> cat.Contains("Stack push/pop"))
        member this.countersCMov                        = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> cat.Contains("Register moves"))
        member this.countersCOthers                     = sumCyclesCategoryCounters this.countersPerAvrOpcodeCategoryNativeC (fun (cat) -> not (cat.Contains("LD/ST rel to")) && not (cat.Contains("Stack push/pop")) && not (cat.Contains("Register moves")))
        member this.countersCTotal                      = this.countersCPushPop + this.countersCMov + this.countersCLoadStore + this.countersCOthers;


    type NmData = {
        address : int;
        size : int;
        symbolType : string;
        name : string;
        file : string; }

    type SimulationResults = {
        benchmark : string;

        passedTestAOT : bool;
        cyclesStopwatchAOT : int;
        cyclesStopwatchC : int;

        countersAOTTimer : ExecCounters;
        countersCTimer : ExecCounters;

        jvmMethods : JvmMethod list;
        cFunctions : CFunction list;

        jvmAllSymbolCounters : (string * ExecCounters) list
        cAllSymbolCounters : (string * ExecCounters) list
        }
        with
        member this.countersPerJvmOpcodeAOTJava         = (this.jvmMethods |> List.collect (fun (jvmMethod) -> jvmMethod.countersPerJvmOpcodeAOTJava) |> sumCountersPerOpcodeAndCategory) @
                                                                                    [ ("14) VM", "", (this.countersAOTVM)) ]
        member this.countersPerAvrOpcodeAOTJava         = (this.jvmMethods |> List.collect (fun (jvmMethod) -> jvmMethod.countersPerAvrOpcodeAOTJava) |> sumCountersPerOpcodeAndCategory) @
                                                                                    [ ("14) VM", "", (this.countersAOTVM)) ]
        member this.countersPerAvrOpcodeNativeC         = (this.cFunctions |> List.collect (fun (cFunction) -> cFunction.countersPerAvrOpcodeNativeC) |> sumCountersPerOpcodeAndCategory)

        member this.countersPerJvmOpcodeCategoryAOTJava = (this.jvmMethods |> List.collect (fun (jvmMethod) -> jvmMethod.countersPerJvmOpcodeCategoryAOTJava) |> sumCountersPerCategory) @
                                                                                    [ ("14) VM", (this.countersAOTVM)) ]
        member this.countersPerAvrOpcodeCategoryAOTJava = (this.jvmMethods |> List.collect (fun (jvmMethod) -> jvmMethod.countersPerAvrOpcodeCategoryAOTJava) |> sumCountersPerCategory) @
                                                                                    [ ("14) VM", (this.countersAOTVM)) ]
        member this.countersPerAvrOpcodeCategoryNativeC = (this.cFunctions |> List.collect (fun (cFunction) -> cFunction.countersPerAvrOpcodeCategoryNativeC) |> sumCountersPerCategory)

        member this.countersCLoadStore                  = this.cFunctions |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersCLoadStore)
        member this.countersCPushPop                    = this.cFunctions |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersCPushPop)
        member this.countersCMov                        = this.cFunctions |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersCMov)
        member this.countersCOthers                     = this.cFunctions |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersCOthers)
        member this.countersCTotal                      =(this.cFunctions |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersCTotal))
        member this.countersCTotalPlusTimer             = this.countersCTotal + this.countersCTimer

        member this.countersAOTLoadStore                = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTLoadStore)
        member this.countersAOTPushPopInt               = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTPushPopInt)
        member this.countersAOTPushPopRef               = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTPushPopRef)
        member this.countersAOTPushPop                  = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTPushPop)
        member this.countersAOTMov                      = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTMov)
        member this.countersAOTOthers                   = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTOthers)
        member this.countersAOTTotal                    =(this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTTotal)) + this.countersAOTVM
        member this.countersAOTTotalPlusTimer           = this.countersAOTTotal + this.countersAOTTimer

        member this.countersJVMLoadStore                = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMLoadStore)
        member this.countersJVMConstantLoad             = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMConstantLoad)
        member this.countersJVMProcessingMath           = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMProcessingMath)
        member this.countersJVMProcessingBitShift       = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMProcessingBitShift)
        member this.countersJVMProcessingBitLogic       = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMProcessingBitLogic)
        member this.countersJVMProcessing               = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMProcessing)
        member this.countersJVMBranches                 = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMBranches)
        member this.countersJVMInvoke                   = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMInvoke)
        member this.countersJVMOthers                   = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMOthers)
        member this.countersJVMTotal                    = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersJVMTotal)

        member this.countersOverheadLoadStore           = this.countersAOTLoadStore - this.countersCLoadStore;
        member this.countersOverheadPushPop             = this.countersAOTPushPop - this.countersCPushPop;
        member this.countersOverheadMov                 = this.countersAOTMov - this.countersCMov;
        member this.countersOverheadOthers              = this.countersAOTOthers - this.countersCOthers;
        member this.countersOverheadVm                  = this.countersAOTVM;
        member this.countersOverheadTotal               = this.countersAOTTotal - this.countersCTotal;

        member this.executedCyclesAOT                   = this.countersAOTTotal.cycles
        member this.executedCyclesC                     = this.countersCTotal.cycles

        member this.cyclesUnaccountedForAOT             = this.cyclesStopwatchAOT - this.countersAOTTotal.cycles
        member this.cyclesUnaccountedForC               = this.cyclesStopwatchC   - this.countersCTotal.cycles

        member this.countersAOTVM                       =
            // Originally we summed the total of these symbols (memset not used in array creation at that time):
            //    [ "RTC_INVOKEVIRTUAL_OR_INTERFACE"; "RTC_INVOKESPECIAL_OR_STATIC_FAST_JAVA"; "RTC_INVOKESTATIC_FAST_NATIVE"; "DO_INVOKEVIRTUAL"; "dj_object_getRuntimeId"; "dj_object_getReferences"; "dj_global_id_mapToInfusion"; "dj_global_id_lookupVirtualMethod"; "dj_vm_getRuntimeClassForInvoke"; "dj_exec_stackPeekDeepRef"; "callJavaMethod_setup"; "callJavaMethod"; "callNativeMethod"; "callMethodFast"; "callMethod"; "dj_infusion_getReferencedInfusionIndex"; "memset" ]
            // but this causes double counting somehow (apparently some of these are used in another call path in the VM somewhere?)
            // We have to compare counters to determine the type of overhead (push/pop, ld/st, other). Since counting the mv method call overhead is too hard, we calculate it as follows:
            //    (stopwatch total - (sum of other counters) - (cycles in timer))
            // This makes the comparison between stopwatch and counter totals meaningless since they will always match, but we've seen enough correct reports now to know we're not missing any methods.
                                                            {
                                                                executions       = 0
                                                                cycles           = this.cyclesStopwatchAOT - (this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.countersAOTTotal)).cycles - this.countersAOTTimer.cycles
                                                                cyclesSubroutine = 0
                                                                count            = 0
                                                                size             = 0
                                                            } 

        member this.maxJvmStackInBytes        =
            this.jvmMethods
            |> List.collect (fun jvmMethod -> jvmMethod.instructions)
            |> List.map (fun jvm -> jvm.djDebugData.stackSizeBefore)
            |> List.max;
        member this.avgJvmStackInBytes        =
            this.jvmMethods
            |> List.collect (fun jvmMethod -> jvmMethod.instructions) 
            |> List.fold (fun (accCnt, accSum) jvm -> (accCnt+jvm.counters.executions, accSum+(jvm.counters.executions*jvm.djDebugData.stackSizeBefore))) (0, 0)
            |> (fun (cnt, sum) -> float sum / float cnt)
        member this.avgJvmStackChangeInBytes  =
            this.jvmMethods
            |> List.collect (fun jvmMethod -> jvmMethod.instructions) 
            |> List.map (fun jvm -> (jvm.counters.executions, jvm.counters.executions * (abs (jvm.djDebugData.stackSizeBefore - jvm.djDebugData.stackSizeAfter))))
            |> List.fold (fun (accCnt, accSum) (jvmCnt, jvmSum) -> (accCnt+jvmCnt, accSum+jvmSum)) (0, 0)
            |> (fun (cnt, sum) -> float sum / float cnt)

        member this.codesizeJavaForAOT                        = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaForAOT)
        member this.codesizeJavaBranchCount                   = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaBranchCount)
        member this.codesizeJavaBranchTargetCount             = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaBranchTargetCount)
        member this.codesizeJavaMarkloopCount                 = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaMarkloopCount)
        member this.codesizeJavaMarkloopTotalSize             = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaMarkloopTotalSize)
        member this.codesizeJavaWithoutBranchOverhead         = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaWithoutBranchOverhead)
        member this.codesizeJavaWithoutBranchMarkloopOverhead = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaWithoutBranchMarkloopOverhead)
        member this.codesizeJavaForInterpreter                = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeJavaForInterpreter)
        member this.codesizeAOT                               = this.jvmMethods |> List.sumBy (fun (jvmMethod) -> jvmMethod.codesizeAOT)
        member this.codesizeC                                 = this.cFunctions |> List.sumBy (fun (cFunction) -> cFunction.codesizeC)

        // SAFETY RELATED CODE
        member this.arrayOrObjectWriteCounters               = this.countersPerJvmOpcodeAOTJava
                                                               |> List.filter (fun (cat, opcode, counters) -> (opcode.StartsWith("JVM_PUTFIELD") || opcode.StartsWith("JVM_PUTARRAY")))
                                                               |> List.map (fun (_, _, counters) -> counters)
                                                               |> List.fold (+) ExecCounters.Zero



        member this.pickleToString =
            let xmlSerializer = FsPickler.CreateXmlSerializer(indent = true, typeConverter = new IgnoreFSI_NamespaceConverter())
            xmlSerializer.PickleToString this
        static member unPickleOfString s =
            let xmlSerializer = FsPickler.CreateXmlSerializer(indent = true, typeConverter = new IgnoreFSI_NamespaceConverter())
            xmlSerializer.UnPickleOfString<SimulationResults> s


