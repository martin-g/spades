#!/bin/sh

RUNNER_PARAM_CABOG_BASE/fastqToCA -insertsize RUNNER_PARAM_INSERT_SIZE RUNNER_PARAM_DEVIATION -type 'RUNNER_PARAM_PHRED_TYPE' -libraryname reads -mates RUNNER_PARAM_LEFT,RUNNER_PARAM_RIGHT >reads.frg 2>>RUNNER_PARAM_STDERR_LOG
RUNNER_PARAM_CABOG_BASE/runCA -d . -p asm -s RUNNER_PARAM_CABOG_CONFIG reads.frg >> RUNNER_PARAM_STDOUT_LOG 2>> RUNNER_PARAM_STDERR_LOG

 
