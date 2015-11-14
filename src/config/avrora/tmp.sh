#!/bin/zsh
alias gdj="gradle -b ../../build.gradle"

gdj clean
gdj avrora_analyse_trace -Prtcbenchmark=bm_sortX
gdj avrora_analyse_trace -Prtcbenchmark=bm_sortO
gdj avrora_analyse_trace -Prtcbenchmark=bm_hsortX
gdj avrora_analyse_trace -Prtcbenchmark=bm_hsorto
gdj avrora_analyse_trace -Prtcbenchmark=bm_fft
gdj avrora_analyse_trace -Prtcbenchmark=bm_xxtea
gdj avrora_analyse_trace -Prtcbenchmark=bm_rc5
gdj avrora_analyse_trace -Prtcbenchmark=bm_binsrchX
gdj avrora_analyse_trace -Prtcbenchmark=bm_binsrchO


