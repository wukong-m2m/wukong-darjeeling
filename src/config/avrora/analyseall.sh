#!/bin/zsh

mono32 process-output/process-output/bin/Debug/process-output.exe process all `pwd`
mono32 process-output/process-output/bin/Debug/process-output.exe combine all `pwd`
