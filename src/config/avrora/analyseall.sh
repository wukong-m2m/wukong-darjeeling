#!/bin/zsh

mono process-output/process-output/bin/Debug/process-output.exe process all `pwd`
mono process-output/process-output/bin/Debug/process-output.exe combine all `pwd`
