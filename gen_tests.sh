#!/bin/bash

GRAPH_DIR="/home/jholten/test_graphs"
OUTPUT_DIR="/home/jholten/perf_tests_kamis_ml"
NUM_MACHINES=2
TEST_NAME=ls_rounds_test


# get current test number
test=0
for i in $OUTPUT_DIR/*; do
    i=$(basename $i)
    (( i > test )) && test=$i
done

(( test++ ))
echo "Running test $TEST_NAME (#$test)"

mkdir $OUTPUT_DIR/$test
mv $OUTPUT_DIR/latest $OUTPUT_DIR/previous
ln -s $OUTPUT_DIR/$test $OUTPUT_DIR/latest

tests_file=$OUTPUT_DIR/$test/$TEST_NAME.txt
echo $tests_file

graphs=$(find $GRAPH_DIR -type f -name "*.graph")

# ml_pruning ratio tests
# for graph in $graphs; do
#     for p in $(seq -f "%.2f" 0.9 0.01 0.99); do 
#         config="echo config ls_rounds 5, ls_time 5, ml_pruning $p"
#         test_call="/home/jholten/KaMIS-ml/deploy/iterative_ml $graph --output=$OUTPUT_DIR/$test/$(basename $graph).mis.ml_pruning_${p#'0.'} --ls_rounds=5 --ls_time=5 --ml_pruning=$p --console_log"
#         echo "$config; $test_call" >> $tests_file
#     done
# done

# ls_time tests
# for graph in $graphs; do
#     for t in 1 2 3 5 7 10 15 20; do 
#         config="echo config ls_rounds 3, ls_time $t, ml_pruning 0.97"
#         test_call="/home/jholten/KaMIS-ml/deploy/iterative_ml $graph --output=$OUTPUT_DIR/$test/$(basename $graph).mis.ls_time_$t --ls_rounds=3 --ls_time=$t --ml_pruning=0.97 --console_log"
#         echo "$config; $test_call" >> $tests_file
#     done
# done

# ls_rounds tests
for graph in $graphs; do
    for r in 1 2 3 5 10; do 
        config="echo config ls_rounds $r, ls_time 2, ml_pruning 0.97"
        test_call="/home/jholten/KaMIS-ml/deploy/iterative_ml $graph --output=$OUTPUT_DIR/$test/$(basename $graph).mis.ls_rounds_$r --ls_rounds=$r --ls_time=2 --ml_pruning=0.97 --console_log"
        echo "$config; $test_call" >> $tests_file
    done
done

total_lines=$(wc -l < $tests_file)
((tests_per_machine = (total_lines + NUM_MACHINES - 1 ) / NUM_MACHINES))
split -a 1 --lines=$tests_per_machine $tests_file $tests_file.
