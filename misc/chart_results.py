import json
import numpy as np
import os
from pprint import pprint
import matplotlib.pyplot as plt


output_directory = "/home/jholten/perf_tests_kamis_ml"
test_number = "27"

# evaluate tests
with open("/home/jholten/KaMIS-ml/optima.txt") as optimum_file:
    split_lines = map(lambda line: line.split(" "), optimum_file.readlines())
    optimum = dict(map(lambda l: (l[0].removeprefix("/home/jholten/test_graphs/"), int(l[1])), split_lines))

ratio = dict()

# has not run before
if not os.path.isfile(f"{output_directory}/{test_number}/results.json"):
    print("Computing MIS size...")
    for output_filename in os.listdir(f"{output_directory}/{test_number}"):
        if output_filename.endswith(".mis"):
            output_filepath = os.path.join(output_directory, test_number, output_filename)
            graph = output_filename.removesuffix(".mis")
            ml = np.loadtxt(output_filepath)
            ratio[graph] = np.sum(ml) / optimum[graph]

    with open(f"{output_directory}/{test_number}/results.json", "w") as out_file:
        json.dump(ratio, out_file)

# has run before, use saved results
else:
    print("Reading saved results")
    with open(f"{output_directory}/{test_number}/results.json") as file:
        ratio = json.load(file)

pprint(ratio)
names, values = zip(*sorted(ratio.items(), key=lambda tup: tup[0]))
names = list(map(lambda s: s.removesuffix("-sorted.graph"), names))
plt.bar(names, values)
plt.show()
