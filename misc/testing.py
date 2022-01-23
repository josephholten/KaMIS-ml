import os
import subprocess
import json
from multiprocessing import Pool

import numpy as np

graph_directory = "/home/jholten/test_graphs"
output_directory = "/home/jholten/perf_tests_kamis_ml"
dir_permissions = 0o774
executable = "/home/jholten/KaMIS-ml/deploy/iterative_ml"


class Config:
    def __init__(self, test_number):
        self.ls_rounds = 5
        self.ls_time = 5
        self.ml_pruning = 0.95
        self.test_number = test_number

    def write_to_file(self):
        path = os.path.join(output_directory, self.test_number, "config.json")
        with open(path, "w") as output_file:
            json.dump(self.__dict__, output_file)


def exec_iterative_ml(config, graph_filename, test_number):
    args = [executable,
            f"{graph_directory}/{graph_filename}",
            f"--output={output_directory}/{test_number}/{graph_filename}.mis",
            f"--ls_rounds={config.ls_rounds}",
            f"--ls_time={config.ls_time}",
            f"--ml_pruning={config.ml_pruning}",
            "--console_log"
            ">",
            f"{output_directory}/{test_number}/{graph_filename}.mis.log"
    ]
    subprocess.run(args)
    print(args)


# check if output_dir exists (if not make it)
if not os.path.exists(output_directory):
    os.mkdir(output_directory, dir_permissions)

# make new folder for tests of current iteration
test_number = str(len(list(os.listdir(output_directory))))
os.mkdir(os.path.join(output_directory, test_number), dir_permissions)
print(f"test #{test_number}")

# run tests
with Pool(processes=os.cpu_count()) as pool:
    for graph_filename in os.listdir(graph_directory):
        graph_filepath = os.path.join(graph_directory, graph_filename)
        if os.path.isfile(graph_filepath):
            pool.apply_async(exec_iterative_ml, (Config(), graph_filename, test_number))

# evaluate tests
with open("/home/jholten/mis/kamis_results/optimum.json") as optimum_file:
    optimum = json.load(optimum_file)

ratio = {}
for output_filename in os.listdir(f"{output_directory}/{test_number}"):
    if output_filename.endswith(".log"):
        output_filepath = os.path.join(output_directory, output_filename)
        with open(output_filepath) as output_file:
            line = filter(lambda s: s.startswith("MIS_weight"), output_file.readlines())
            weight = line.split(" ")[1]
            graph = output_filename[:output_filename.find(".")]
            ratio[graph] = weight / optimum[graph]

print(ratio)




