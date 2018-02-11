#!/bin/zsh

curdir=`pwd`


# Result sets from runall.sh
for resultset in `ls -d resultsets_*`
do
    resultsetfull="${curdir}/${resultset}"
    echo ${resultsetfull}
    mono process-output/process-output/bin/Debug/process-output.exe process all ${resultsetfull}
    mono process-output/process-output/bin/Debug/process-output.exe combine all ${resultsetfull}
done

# Any results in main directory
mono process-output/process-output/bin/Debug/process-output.exe process all ${curdir}
mono process-output/process-output/bin/Debug/process-output.exe combine all ${curdir}
