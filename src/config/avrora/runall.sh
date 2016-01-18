#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

# benchmarks=(sortX hsortX binsrchX)
benchmarks=(bsort hsort fft xxtea rc5 md5 binsrch)

gdj clean

# BASELINE
for benchmark in ${benchmarks}
do
    gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=baseline
done

# SIMPLE STACK CACHING
for benchmark in ${benchmarks}
do
	# cachesizes=(5 6 7 8 9 10 11)
	cachesizes=(5 10 11)
	for aotstackcachesize in ${cachesizes}
	do
	    gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache -Paotstackcachesize=${aotstackcachesize}
	done
done

# POPPED STACK CACHING
for benchmark in ${benchmarks}
do
    # cachesizes=(5 6 7 8 9 10 11)
    cachesizes=(11)
    for aotstackcachesize in ${cachesizes}
    do
        gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache -Paotstackcachesize=${aotstackcachesize}
    done
done

# POPPED STACK CACHING + MARKLOOP
for benchmark in ${benchmarks}
do
    markloopregs=(1 2 3 4 5 6 7)
    for aotmarkloopregs in ${markloopregs}
    do
        gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=markloop -Paotstackcachesize=11 -Paotmarkloopregs=${aotmarkloopregs}
    done
done

for resultsdir in `ls | grep results_`
do
	fsharpi --exec proces-trace/proces-trace/CombineResults.fsx "./${resultsdir}"
done



