#!/bin/zsh

alias gdj="gradle -b ../../build.gradle"

benchmarks=(bsort32 hsort32 binsrch32 fft xxtea rc5 md5)

gdj clean

for benchmark in ${benchmarks}
do
    gdj avrora_store_trace -Paotbm=${benchmark} -Paotstrat=markloop -Paotstackcachesize=11 -Paotmarkloopregs=7 -Paotconstshiftoptimisation=gcc_like
done

./analyseall.sh

