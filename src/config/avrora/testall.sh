#!/bin/zsh

alias gdj="gradle -b ../../build.gradle"

benchmarks=(bsort32 hsort32 binsrch32 fft xxtea rc5 md5)

gdj clean --offline

for benchmark in ${benchmarks}
do
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=gcc_like --offline
done

./analyseall.sh

head -n 2 results_4MARK_R11_P7_CS4_A16/summary_4MARK_R11_P7_CS4_A16
