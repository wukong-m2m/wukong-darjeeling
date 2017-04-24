#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

# benchmarks=(sortX hsortX binsrchX)
# benchmarks=(bsort16 bsort32 hsort16 hsort32 binsrch16 binsrch32 fft xxtea rc5 md5)
benchmarks=(bsort32 hsort32 binsrch32 fft xxtea rc5 md5 coremk)

gdj clean

# test
# # BASELINE, plus 16 bit array index: different constant optimisation strategies
# for benchmark in ${benchmarks}
# do
#     constshifts=(none by1 all_only_shift all_move_and_shift gcc_like)
#     for aotconstshiftoptimisation in ${constshifts}
#     do
#         gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=baseline -Paotconstshiftoptimisation=${aotconstshiftoptimisation}
#     done
# done

## All others use default "gcc_like" constant shift optimisation

# BASELINE
for benchmark in ${benchmarks}
do
    gdj avrora_store_trace     -Paotbm=${benchmark} -Paotstrat=baseline         -Paotstackcachesize=0                    -Paotmarkloopregs=0                  -Paotconstshiftoptimisation=none
done

# IMPROVED PEEPHOLE OPTIMISER
for benchmark in ${benchmarks}
do
  gdj avrora_store_trace       -Paotbm=${benchmark} -Paotstrat=improvedpeephole -Paotstackcachesize=0                    -Paotmarkloopregs=0                  -Paotconstshiftoptimisation=none
done

# SIMPLE STACK CACHING
for benchmark in ${benchmarks}
do
	# cachesizes=(5 6 7 8 9 10 11)
	cachesizes=(11)
	for aotstackcachesize in ${cachesizes}
	do
	    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache -Paotstackcachesize=${aotstackcachesize} -Paotmarkloopregs=0                  -Paotconstshiftoptimisation=none
	done
done

# POPPED STACK CACHING
for benchmark in ${benchmarks}
do
    # cachesizes=(5 6 7 8 9 10 11)
    cachesizes=(11)
    for aotstackcachesize in ${cachesizes}
    do
        gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache -Paotstackcachesize=${aotstackcachesize} -Paotmarkloopregs=0                  -Paotconstshiftoptimisation=none
    done
done

# MARK LOOPS
for benchmark in ${benchmarks}
do
    # markloopregs=(1 2 3 4 5 6 7)
    markloopregs=(7)
    for aotmarkloopregs in ${markloopregs}
    do
        gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Paotstackcachesize=11                   -Paotmarkloopregs=${aotmarkloopregs} -Paotconstshiftoptimisation=none
    done
done

# CONST SHIFT
for benchmark in ${benchmarks}
do
    constshifts=(none gcc_like)
    # constshifts=(none by1 all_only_shift all_move_and_shift gcc_like)
    for aotconstshiftoptimisation in ${constshifts}
    do
        gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Paotstackcachesize=11                   -Paotmarkloopregs=7                  -Paotconstshiftoptimisation=${aotconstshiftoptimisation} # -Paot32bitindex=true
    done
done

# MARKLOOP+CONST SHIFT FOR DIFF NUMBERS OF PINNED REGISTERS
for benchmark in ${benchmarks}
do
    markloopregs=(1 2 3 4 5 6 7)
    for aotmarkloopregs in ${markloopregs}
    do
        gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop         -Paotstackcachesize=11                   -Paotmarkloopregs=${aotmarkloopregs} -Paotconstshiftoptimisation=gcc_like
    done
done

# Special case: CoreMark is too big to fit both Java and C versions in memory at the same time. Run the coremk_c config for the native CoreMark results, and then copy the result here.
cd ../coremk_c
rm -rf results_coremk_c
gdj avrora_store_trace
cd -
rm -rf results_coremk_c
cp -r ../coremk_c/results_coremk_c .

./analyseall.sh



