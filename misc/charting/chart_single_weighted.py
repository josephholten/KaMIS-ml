import json

import matplotlib
import sys

import numpy as np
import os
from pprint import pprint
import matplotlib.pyplot as plt

DEFAULT_CONFIG = {"ls_rounds": 2, "ls_time": 2, "ml_pruning": 0.97}

output_directory = "/home/jholten/perf_tests_kamis_ml"
test_name = "weighted"
config = DEFAULT_CONFIG

# evaluate tests
with open("/home/jholten/weighted_optima.txt") as optimum_file:
    split_lines = map(lambda line: line.split(" "), optimum_file.readlines())
    optimum = dict(map(lambda l: (l[0][len("/home/jholten/weighted_test_graphs/"):], int(l[1])), split_lines))

with open("/home/jholten/KaMIS-ml/weighted_test_graphs_iterative_ml.txt") as results_file:
    split_lines = map(lambda line: line.split(" "), results_file.readlines())
    results = dict(map(lambda l: (l[0][len("/home/jholten/weighted_test_graphs/"):], int(l[1])), split_lines))


graphing_data = {}

for graph in results:
    print(graph, results[graph] / optimum[graph])