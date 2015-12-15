#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

# benchmarks=(sortX sortO hsortX hsortO fft xxtea rc5 md5 binsrchX binsrchO)
benchmarks=(sortO hsortO fft xxtea rc5 md5 binsrchO)

gdj clean

# BASELINE
for benchmark in ${benchmarks}
do
    gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=baseline
done

# SIMPLE STACK CACHING
for benchmark in ${benchmarks}
do
	cachesizes=(5 6 7 8 9 10)
	for aotstackcachesize in ${cachesizes}
	do
	    gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=simplestackcaching -Paotstackcachesize=${aotstackcachesize}
	done
done

for resultsdir in `ls | grep results_`
do
	fsharpi --exec proces-trace/proces-trace/CombineResults.fsx "./${resultsdir}"
done



