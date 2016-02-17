#!/bin/zsh
rm -f sumsum.txt

touch sumsum.txt

summaries=
summaries="$summaries "`ls results_baseline*/summary*`
summaries="$summaries "`ls results_simple*/summary*`
summaries="$summaries "`ls results_popped*/summary*`
summaries="$summaries "`ls results_markloop*/summary*`

for summary in `echo $summaries`
do
    splitfilename=("${(ws:/:)summary}")
    echo ${splitfilename[1]} >> sumsum.txt
    head -n 1 ${summary} | tail -n 1 >> sumsum.txt
    if [ $# -eq 0 ]; then
        head -n 2 ${summary} | tail -n 1 >> sumsum.txt
        head -n 8 ${summary} | tail -n 1 >> sumsum.txt
        head -n 9 ${summary} | tail -n 1 >> sumsum.txt
        head -n 10 ${summary} | tail -n 1 >> sumsum.txt
    else
        for line in "$@"
        do
            head -n ${line} ${summary} | tail -n 1 >> sumsum.txt
        done
    fi
done

cat sumsum.txt