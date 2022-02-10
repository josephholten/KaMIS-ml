#!/bin/bash

GRAPH_FOLDER="/home/jholten/amazon_instances"
OUT_FOLDER="/home/jholten/mis/kamis_results"

parallel --progress --bar -j 7 --joblog gen_optimum_amazon.log /home/jholten/KaMIS/deploy/weighted_branch_reduce $GRAPH_FOLDER/{} --output=$OUT_FOLDER/{}.mis --time_limit=43200 ::: $(ls $GRAPH_FOLDER) | tee amazon_kamis_log.txt

