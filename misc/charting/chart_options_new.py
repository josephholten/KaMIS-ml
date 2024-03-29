import json
import sys

import numpy as np
import os
from pprint import pprint
import matplotlib.pyplot as plt


def parse(name, s):
    if s.startswith(name + " "):
        return s[len(name + " "):]
    else:
        return None


output_directory = "/home/jholten/perf_tests_kamis_ml"
test_name = "ml_pruning_low"
test_option = "ml_pruning"

# evaluate tests
with open("/home/jholten/KaMIS-ml/optima.txt") as optimum_file:
    split_lines = map(lambda line: line.split(" "), optimum_file.readlines())
    optimum = dict(map(lambda l: (l[0][len("/home/jholten/test_graphs/"):], int(l[1])), split_lines))
    pprint(optimum)

log = []

# has not run before
with open(f"{output_directory}/{test_name}/log.txt") as log_file:
    config = dict()
    info = {"time": None, "ratio": None, "MIS_weight": None, "ls_std_dev": []}
    graph = ""
    for line in log_file:
        line = line.strip()

        # clear line and have collection information
        if not line and graph and config:
            log.append({"config": config, "graph": graph, "info": info})
            config = dict()
            info = {"time": None, "ratio": None, "MIS_weight": None, "ls_std_dev": []}
            graph = ""

        if value := parse("config", line):
            options = list(map(lambda s: s.strip(), value.split(", ")))
            for field, option in zip(["ls_rounds", "ls_time", "ml_pruning"], options):
                config[field] = float(parse(field, option))

        if value := parse("graph", line):
            graph = value.strip()

        if value := parse("MIS_weight", line):
            info["MIS_weight"] = int(value)
            info["ratio"] = int(value) / optimum[graph]

        if value := parse("reduction_time", line):
            info["time"] = float(value)

        if value := parse("ls_std_dev", line):
            info["ls_std_dev"].append(float(value))

        # with open(f"{output_directory}/{test_number}/info.json", "w") as info_file:
        #     json.dump(info, info_file)

pprint(log)

graphing_data = {}

# chart ratio over pruning
# group by "graph"
for test in log:
    if not test["graph"] in graphing_data:
        graphing_data[test["graph"]] = {'x': [test["config"][test_option]],
                                    'ratio': [test["info"]["ratio"]],
                                    'time': [test["info"]["time"]],
                                    'label': test["graph"][:-len("-sorted.graph")]}
    else:
        graphing_data[test["graph"]]['x'].append(test["config"][test_option])
        graphing_data[test["graph"]]['ratio'].append(test["info"]["ratio"])
        graphing_data[test["graph"]]['time'].append(test["info"]["time"])

plt.subplot(211)
for graph, data in graphing_data.items():
    plt.plot(data['x'], data['ratio'], label=data["label"])

#plt.yticks(np.linspace(0.70, 1.00, num=20))
# plt.xticks([1,2,3,5,10])
plt.title(f"very aggressive pruning")
plt.yscale('log')
plt.ylabel("KaMIS-ml(G)/$\\alpha$(G)")

plt.legend(bbox_to_anchor=(1,1), loc="upper left")

plt.subplot(212)

for graph, data in graphing_data.items():
    plt.plot(data['x'], data['time'], label=data["label"])


#plt.legend()
plt.tight_layout()
plt.yscale('log')
plt.ylabel("t")

plt.subplots_adjust(right=0.70, hspace=0)

plt.savefig(f"{test_name}_tests.pdf", dpi=(300))
#plt.show()

# has run before, use saved results
# else:
#     print("Reading saved results")
#     with open(f"{output_directory}/{test_number}/info.json") as file:
#         info = json.load(file)
#
# for key, value in list(info.items()):
#     if not value:
#         del info[key]

# names, values = zip(*sorted(info.items(), key=lambda tup: tup[0].lower()))
# names = list(map(lambda s: s[:len("-sorted.graph")], names))
# plt.bar(names, list(map(lambda x: x["ratio"], values)))
# plt.show()
