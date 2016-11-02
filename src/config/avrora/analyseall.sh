#!/bin/zsh

fsharpi --exec proces-trace/proces-trace/ProcessTraces.fsx all `pwd`
fsharpi --exec proces-trace/proces-trace/CombineResults.fsx all `pwd`
