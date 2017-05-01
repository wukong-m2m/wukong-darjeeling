#!/bin/zsh
rm -f sumsum.txt

touch sumsum.txt

summaries="$summaries "`ls results_*/summary*`

for summary in `echo $summaries`
do
    splitfilename=("${(ws:/:)summary}")
    echo ${splitfilename[1]} >> sumsum.txt
    head -n 1 ${summary} | tail -n 1 >> sumsum.txt
    if [ $# -eq 0 ]; then
        head -n 2 ${summary} | tail -n 1 >> sumsum.txt
        head -n 4 ${summary} | tail -n 1 >> sumsum.txt
        head -n 6 ${summary} | tail -n 1 >> sumsum.txt
        head -n 12 ${summary} | tail -n 1 >> sumsum.txt
        head -n 13 ${summary} | tail -n 1 >> sumsum.txt
    else
        for line in "$@"
        do
            head -n ${line} ${summary} | tail -n 1 >> sumsum.txt
        done
    fi
done

cat sumsum.txt