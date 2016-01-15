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
	# cachesizes=(5 6 7 8 9 10)
	cachesizes=(5 10)
	for aotstackcachesize in ${cachesizes}
	do
	    gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=simplestackcache -Paotstackcachesize=${aotstackcachesize}
	done
done

# POPPED STACK CACHING
for benchmark in ${benchmarks}
do
    # cachesizes=(5 6 7 8 9 10)
    cachesizes=(10)
    for aotstackcachesize in ${cachesizes}
    do
        gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=poppedstackcache -Paotstackcachesize=${aotstackcachesize}
    done
done

# POPPED STACK CACHING + MARKLOOP
for benchmark in ${benchmarks}
do
    # cachesizes=(5 6 7 8 9 10)
    cachesizes=(10)
    for aotstackcachesize in ${cachesizes}
    do
        gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=markloop -Paotstackcachesize=${aotstackcachesize}
    done
done

for resultsdir in `ls | grep results_`
do
	fsharpi --exec proces-trace/proces-trace/CombineResults.fsx "./${resultsdir}"
done



