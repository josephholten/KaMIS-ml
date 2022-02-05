#!/bin/bash

GRAPH_DIR="/home/jholten/test_graphs"
OUTPUT_DIR="/home/jholten/perf_tests_kamis_ml"

# get current test number
test=0
for i in $(ls $OUTPUT_DIR); do
    (( i > test )) && test=$i
done

let "test++"
echo "Running test #$test"

mkdir $OUTPUT_DIR/$test
mv $OUTPUT_DIR/latest $OUTPUT_DIR/previous
ln -s $OUTPUT_DIR/$test $OUTPUT_DIR/latest

graphs=$(find $GRAPH_DIR)

# run tests in parallel
parallel --progress --bar deploy/iterative_ml {} --output=$OUTPUT_DIR/$test/{/}.mis --ls_rounds=5 --ls_time=5 --ml_pruning=0.98 --console_log ::: $graphs | tee $OUTPUT_DIR/$test/log.txt
