import json
from pprint import pprint

RANDOMIZATION_METHOD = "hybrid"
LOG_PATH = f"/home/jholten/perf_tests_kamis_ml/optimum_weight_averaging_{RANDOMIZATION_METHOD}/log.txt"
NUM_SEEDS = 5


def parse(name, s):
    if s.startswith(name + " "):
        return s[len(name + " "):]
    else:
        return None


optima = {}
with open(LOG_PATH) as log_file:
    graph = ""
    for line in log_file:
        line = line.strip()
        if line.endswith(".graph"):
            graph = line
        if value := parse("MIS_weight", line):
            if graph not in optima:
                optima[graph] = int(value)
            else:
                optima[graph] += int(value)

for graph in optima.keys():
    optima[graph] /= NUM_SEEDS

with open(f"/home/jholten/KaMIS-ml/optima_weighted_long_{RANDOMIZATION_METHOD}.txt", "w") as output:
    json.dump(optima, output)

pprint(optima)
