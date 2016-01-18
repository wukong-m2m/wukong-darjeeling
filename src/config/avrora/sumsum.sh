#!/bin/zsh
rm -f sumsum.txt

touch sumsum.txt

for summary in `ls results_*/summary*`
do
    echo >> sumsum.txt
    echo ${summary} >> sumsum.txt
    head -n 1 ${summary} | tail -n 1 >> sumsum.txt
    head -n 2 ${summary} | tail -n 1 >> sumsum.txt
    head -n 8 ${summary} | tail -n 1 >> sumsum.txt
    head -n 9 ${summary} | tail -n 1 >> sumsum.txt
done

cat sumsum.txt