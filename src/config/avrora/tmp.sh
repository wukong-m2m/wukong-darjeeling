#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_sort
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_sorto
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_hsort
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_hsorto
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_fft
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_xxtea
gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_rc5


