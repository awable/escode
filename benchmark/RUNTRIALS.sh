#!/usr/bin/env bash

_BENCHDATE=$(date +"%Y-%m-%d_%H.%M.%S")
echo "Creating Directory $_BENCHDATE"
mkdir $_BENCHDATE
for run in {1..10}; do ./benchmark.py $_BENCHDATE/_TRIALS_$run; done
echo "Merging Trial Files"
cat $_BENCHDATE/_TRIALS_* | sort > $_BENCHDATE/TRIALS
rm $_BENCHDATE/_TRIALS_*
