#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

function store_as_resultset() {
	rm -rf $1
    mkdir -p $1
    mv results_* $1
    cp -r $1/results_coremk_c . # Copy this back for the next set, which may or may not include CoreMark
    cp -r $1/results_motetrack_c . # Copy this back for the next set, which may or may not include CoreMark
}

##### Preparation
# Clean old results
gdj clean
mkdir -p oldresults
mv results_* oldresults
# CoreMark and MoteTrack are too big to fit both Java and C versions in memory at the same time. Run the coremk_c and motetrack_c configs for the native results, and then copy the result here.
cd ../coremk_c
rm -rf results_coremk_c
gdj avrora_store_trace
cd -
rm -rf results_coremk_c
cp -r ../coremk_c/results_coremk_c .

cd ../motetrack_c
rm -rf results_motetrack_c
gdj avrora_store_trace
cd -
rm -rf results_motetrack_c
cp -r ../motetrack_c/results_motetrack_c .


##### Main benchmarks
benchmarks=(bsort16 hsort16 binsrch16 xxtea md5 rc5 coremk motetrack heat_calib heat_detect lec fft16 outlier16u)




##### BASELINE: Both baselines have GET/PUTFIELD_A_FIXED turned on, since this optimisation compensates for Darjeeling specific overhead, so we shouldn't include it in the overhead from Joshua's work.
# UNOPTIMISED COREMARK
gdj avrora_store_trace -Paotbm=coremk_or0       -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false -Pusegetfield_a_fixed=true
# OPTIMISED JAVA
for benchmark in ${benchmarks}
do
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false -Pusegetfield_a_fixed=true
done
store_as_resultset resultsets_baseline




##### MAIN GRAPHS
for benchmark in ${benchmarks}
do
    # Lightweight methods and GET/PUTFIELD_A_FIXED are turned on for these.
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true

    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=improvedpeephole -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true

    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=false -Puselightweightmethods=true  -Pusegetfield_a_fixed=true
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Puseconstantshiftoptimisation=true  -Puse16bitarrayindex=true  -Pusesimul=true  -Puselightweightmethods=true  -Pusegetfield_a_fixed=true
done
store_as_resultset resultsets_maingraphs




##### SAFETY
safetybenchmarks=($benchmarks fillarray8 fillarray16 fillarray32)
for benchmark in ${safetybenchmarks}
do
    # UNSAFE
    gdj avrora_store_trace -Paotbm=${benchmark} # Already done in main graphs part.
    # SAFE
    gdj avrora_store_trace -Paotbm=${benchmark} -Psafe
    # SAFE READS
    gdj avrora_store_trace -Paotbm=${benchmark} -Psafer
done
store_as_resultset resultsets_safety




##### MARKLOOP + XXTEA FOR DIFF NUMBERS OF PINNED REGISTERS
markloopregs=(1 2 3 4 5 6 7)
for benchmark in ${benchmarks}
do
    for aotmarkloopregs in ${markloopregs}
    do
        gdj avrora_store_trace -Paotbm=${benchmark} -Paotmarkloopregs=${aotmarkloopregs}
    done
done
store_as_resultset resultsets_markloop_pinnedregs




##### 8/16/32 bit comparison
bitcomparisonbenchmarks=(bsort8 hsort8 binsrch8 outlier8u fft8 bsort16 hsort16 binsrch16 outlier16u fft16 bsort32 hsort32 binsrch32 outlier32u fillarray8 fillarray16 fillarray32)
for benchmark in $bitcomparisonbenchmarks
do
    gdj avrora_store_trace -Paotbm=${benchmark}
done
store_as_resultset resultsets_8_16_32_bit





##### CoreMark manual optimisations
# Absolute baseline: not optimised code, no lightweight methods, baseline translation, no instructionset optimisations (same as above)
gdj avrora_store_trace -Paotbm=coremk_or0       -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=false -Pusesimul=false -Puselightweightmethods=false -Pusegetfield_a_fixed=true
# Using our optimisations the original code can get as good as:
gdj avrora_store_trace -Paotbm=coremk_or0
# Step by step manual optimisation
gdj avrora_store_trace -Paotbm=coremk_or1
gdj avrora_store_trace -Paotbm=coremk_or2
gdj avrora_store_trace -Paotbm=coremk_or3
gdj avrora_store_trace -Paotbm=coremk_or4
gdj avrora_store_trace -Paotbm=coremk_or5
# Should be identical to normal CoreMark version
gdj avrora_store_trace -Paotbm=coremk
# Two 'unfair' optimisations
gdj avrora_store_trace -Paotbm=coremk_ch1
gdj avrora_store_trace -Paotbm=coremk_ch2
store_as_resultset resultsets_coremark_optimisations




#### Breakdown of executed JVM instructions (executions, not cycles. for the top of the performance table). Get these with 16 bit indexes enabled, otherwise we get lots of I2S conversions since the code uses shorts.
for benchmark in ${benchmarks}
do
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline         -Puseconstantshiftoptimisation=false -Puse16bitarrayindex=true -Pusesimul=false
done
store_as_resultset resultsets_jvm_execution_count




##### Lightweight methods
# CoreMark
gdj avrora_store_trace -Paotbm=coremk 
gdj avrora_store_trace -Paotbm=coremk_lw     # Coremk with ee_isdigit as a lightweight instead of inlined
gdj avrora_store_trace -Paotbm=coremk_fn     # Coremk with only methods (but ee_isdigit still inlined, coremk_fn has ee_isdigit as a normal method as well)
# FFT
gdj avrora_store_trace -Paotbm=fft16
gdj avrora_store_trace -Paotbm=fft16_lw      # FFT with FIX__MPY as a hardcoded Lightweight method, and SIN8/COS8 inlined by ProGuard
gdj avrora_store_trace -Paotbm=fft16_fn      # FFT with FIX__MPY as a normal method, and SIN8/COS8 inlined by ProGuard
# Heap sort
gdj avrora_store_trace -Paotbm=hsort16
gdj avrora_store_trace -Paotbm=hsort16_fn    # Heap sort with siftDown as a normal method
gdj avrora_store_trace -Paotbm=hsort16_cht   # Heap sort with siftDown manually inlined
# Heap sort
gdj avrora_store_trace -Paotbm=hsort32
gdj avrora_store_trace -Paotbm=hsort32_fn    # Heap sort with siftDown as a normal method
gdj avrora_store_trace -Paotbm=hsort32_cht   # Heap sort with siftDown manually inlined
store_as_resultset resultsets_lightweight_methods




##### Number of available registers
stackcachesizes=(4 5 6 7 8 9 10 11)
for benchmark in ${benchmarks}
do
    for aotstackcachesize in ${stackcachesizes}
    do
        echo $aotstackcachesize
        gdj avrora_store_trace -Paotbm=${benchmark} -Paotstackcachesize=${aotstackcachesize}
    done
done
store_as_resultset resultsets_stackcachesize




./analyseall.sh



