import os
import subprocess
import json
from multiprocessing import Pool

import numpy as np



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




# run tests
with Pool(processes=os.cpu_count()) as pool:
    for graph_filename in os.listdir(graph_directory):
        graph_filepath = os.path.join(graph_directory, graph_filename)
        if os.path.isfile(graph_filepath):
            pool.apply_async(exec_iterative_ml, (Config(), graph_filename, test_number))

