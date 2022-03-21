import json

import matplotlib
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


def parse_log_single(file_path):
    log = []
    with open(file_path) as log_file:
        info = {"time": None, "ratio": None, "MIS_weight": None, "ls_std_dev": []}
        graph = ""
        skip = False

        for line in log_file:
            line = line.strip()

            if value := parse("config", line):
                if "ls_rounds 2, ls_time 2, ml_pruning 0.97" in value:
                    skip = True
                else:
                    skip = False

            # clear line and have collection information
            if not line and graph:
                log.append({"graph": graph, "info": info})
                info = {"time": None, "ratio": None, "MIS_weight": None, "ls_std_dev": []}
                graph = ""

            if not skip:
                if value := parse("graph", line):
                    graph = value.strip()

                if value := parse("MIS_weight", line):
                    info["MIS_weight"] = int(value)
                    info["ratio"] = int(value) / optimum[graph]

                if value := parse("reduction_time", line):
                    info["time"] = float(value)

                if value := parse("ls_std_dev", line):
                    info["ls_std_dev"].append(float(value))
    return log


DEFAULT_CONFIG = {"ls_rounds": 2, "ls_time": 2, "ml_pruning": 0.97}

output_directory = "/home/jholten/perf_tests_kamis_ml"
test_name = "weighted_correct"
config = DEFAULT_CONFIG

# evaluate tests
with open("/home/jholten/KaMIS-ml/optima_weighted_long_uniform.txt") as optimum_file:
    optimum = json.load(optimum_file)
    pprint(optimum)

# if not os.path.isfile(f"{output_directory}/{test_name}/info.json"):

log = parse_log_single(f"{output_directory}/{test_name}/log.txt")

graphing_data = {}

# chart ratio & time
# group by "graph"
for test in log:
    entry = {'ratio_normal': test["info"]["ratio"],
             'time_normal': test["info"]["time"],
             'label': test["graph"][:-len("-sorted.graph")]}
    if test["graph"] not in graphing_data:
        graphing_data[test["graph"]] = [entry]
    else:
        graphing_data[test["graph"]].append(entry)

for graph, tests in graphing_data.keys():
    graphing_data[graph] = {'ratio_normal': sum([entry["ratio_normal"] for entry in tests]) / len(tests),
                            'time_normal': sum([entry["time_normal"] for entry in tests]) / len(tests),
                            'label': tests[0]["label"]}

bar_width = 0.33
x = np.arange(len(graphing_data))

fig, axes = plt.subplots(2, 1)

fig.suptitle("training on mtx test graphs only")

p1 = axes[0].bar(x, [data['ratio_normal'] for data in graphing_data.values()], label="Naive model", width=bar_width)
axes[0].bar_label(p1)

axes[0].set_ylabel("KaMIS-ml(G)/$\\alpha$(G)")
axes[0].tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)

p2 = axes[1].bar(x, [data['time_normal'] for data in graphing_data.values()], label="Naive", width=bar_width)
axes[1].bar_label(p2)

axes[1].set_xticks(x, [data['label'] for data in graphing_data.values()], rotation=315, horizontalalignment="left")
axes[1].set_yscale('log')
axes[1].set_ylabel('t')

axes[1].set_title(f"")

#axes[1].legend()

plt.subplots_adjust(hspace=0.1, bottom=0.25)
plt.savefig(f"{test_name}_tests.pdf", dpi=(300))
plt.show()

# has run before, use saved results
# else:
#     print("Reading saved results")
#     with open(f"{output_directory}/{test_name}/info.json") as file:
#         info = json.load(file)
#
# for key, value in list(info.items()):
#     if not value:
#         del info[key]

# names, values = zip(*sorted(info.items(), key=lambda tup: tup[0].lower()))
# names = list(map(lambda s: s[:len("-sorted.graph")], names))
# plt.bar(names, list(map(lambda x: x["ratio"], values)))
# plt.show()
