#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

## declare an array variable
benchmarks=(sortX sortO hsortX hsortO fft xxtea rc5 md5 binsrchX binsrchO)
aot_strategies=(baseline simplestackcaching)

## now loop through the above array
for aot_strategy in ${aot_strategies}
do
    for benchmark in ${benchmarks}
    do
        echo "${benchmark} ${aot_strategy}"
        gdj avrora_analyse_trace -Paotbm=${benchmark} -Paotstrat=${aot_strategy}
    done
    fsharpi --exec proces-trace/proces-trace/CombineResults.fsx ./results_${aot_strategy}
done



