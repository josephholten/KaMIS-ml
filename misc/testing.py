import os
import subprocess
import json
from dataclasses import dataclass

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

    def __enter__(self):
        print("set up config as", self)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        print("serializing")
        self.write_to_file()

    def __str__(self):
        return str(self.__dict__)


def exec_iterative_ml(config, graph_filename):
    args = [executable,
            f"{graph_directory}/{graph_filename}",
            f"--output={output_directory}/{test_number}/{graph_filename}.mis",
            f"--ls_rounds={config.ls_rounds}",
            f"--ls_time={config.ls_time}",
            f"--ml_pruning={config.ml_pruning}",
    ]
    # subprocess.run(args)
    print(args)


if __name__ == "__main__":
    # check if output_dir exists (if not make it)
    if not os.path.exists(output_directory):
        os.mkdir(output_directory, dir_permissions)

    # make new folder for tests of current iteration
    test_number = str(len(list(os.listdir(output_directory))))
    os.mkdir(os.path.join(output_directory, test_number), dir_permissions)
    print(f"test #{test_number}")

    with Config(test_number) as config:
        for graph_filename in os.listdir(graph_directory):
            graph_filepath = os.path.join(graph_directory, graph_filename)
            if os.path.isfile(graph_filepath):
                exec_iterative_ml(config, graph_filename)