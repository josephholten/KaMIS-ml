import json
import sys

import numpy as np
import os
from pprint import pprint
import matplotlib.pyplot as plt


def parse(name):
    if line.startswith(name + " "):
        return line[len(name + " "):]
    else:
        return None


output_directory = "/home/jholten/perf_tests_kamis_ml"
test_number = "32"

# evaluate tests
with open("/home/jholten/KaMIS-ml/optima.txt") as optimum_file:
    split_lines = map(lambda line: line.split(" "), optimum_file.readlines())
    optimum = dict(map(lambda l: (l[0][len("/home/jholten/test_graphs/"):], int(l[1])), split_lines))
    pprint(optimum)

info = dict()

# has not run before
if not os.path.isfile(f"{output_directory}/{test_number}/info.json"):
    if os.path.isfile(f"{output_directory}/{test_number}/log.txt"):
        with open(f"{output_directory}/{test_number}/log.txt") as log_file:
            current_info = dict()
            current_graph = ""
            for line in log_file:
                if value := parse("graph"):
                    current_graph = value.strip()
                    info[current_graph] = {}
                    current_info = info[current_graph]

                if value := parse("MIS_weight"):
                    current_info["MIS_weight"] = int(value)
                    current_info["ratio"] = int(value) / optimum[current_graph]

                if value := parse("reduction_time"):
                    current_info["time"] = float(value)

                if value := parse("ls_std_dev"):
                    try:
                        current_info["ls_std_dev"].append(float(value))
                    except KeyError as error:
                        current_info["ls_std_dev"] = [float(value)]
        with open(f"{output_directory}/{test_number}/info.json", "w") as info_file:
            json.dump(info, info_file)
    else:
        print("no log to parse, exiting ...")
        sys.exit(1)

# has run before, use saved results
else:
    print("Reading saved results")
    with open(f"{output_directory}/{test_number}/info.json") as file:
        info = json.load(file)

pprint(info)
names, values = zip(*sorted(info.items(), key=lambda tup: tup[0]))
names = list(map(lambda s: s[:len("-sorted.graph")], names))
plt.title("Ratios to optimum")
plt.bar(names, list(map(lambda x: x["ratio"], values)))
plt.show()
