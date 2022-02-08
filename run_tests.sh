#!/bin/bash
parallel -k --progress --bar :::: $1 | tee log.txt
