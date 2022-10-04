# KaMIS-ml #

## Description ##
This is a fork of KaMIS v2.0, adding Reduce-and-Peel algorithms, using machine learning heuristics.
The machine learning uses XGBoost and TensorFlow.

## Installation ##
Follow the installation for KaMIS and update the git submodules.

## Usage ##
This gives a brief overview of the usage and options for each executable.
For the executables provided by KaMIS, see `kamis_readme.md`

### greedy_reduction ###
Perform Reduce-and-peel using the engineered heuristic, see below.

`greedy_reduction FILE [options]`

`FILE` The graph to apply the algorithm to.

The options are:

- `--console_log`
Write the log to the console.

- `--output=<string>`
Path to store the resulting independent set.
If not otherwise specified, a log file is produced.
It is saved under the path, where the last file extension is stripped off the output string, and `.log` is appended.

- `--seed=<int>`
Seed to use for the random number generator.

- `--time_limit=<double>`
Time limit until the algorithm terminates.

- `--weight_source=<string>`
Choose how the weights are assigned. Can be either: file (default), hybrid, uniform, geometric.

- `--log_file=<string>`
The path where to save the log file to. The log file is in the JSON format.

### iterative_ml ###
Perform Reduce-and-peel using a based machine learning heuristic.

`iterative_ml FILE [options]`

The options are:

- `--model=<string>` The path to the machine learning model to use. 

- `--nn` Switch from the default random-forest heuristic to neural network based one. 
Use a corresponding model.

- `--console_log`
Write the log to the console.

- `--output=<FILE>`
Path to store the resulting independent set.
If not otherwise specified, a log file is produced.
It is saved under the path, where the last file extension is stripped off the output string, and `.log` is appended.

- `--seed=<int>`
Seed to use for the random number generator.

- `--time_limit=<double>`
Time limit until the algorithm terminates.

- `--weight_source=<string>`
Choose how the weights are assigned. Can be either: file (default), hybrid, uniform, geometric.

- `--log_file=<string>`
The path where to save the log file to. The log file is in the JSON-format.

### simple_greedy ###
Perform a greedy algorithm, prioritising based on the engineered heuristic.
No reductions are performed.

`simple_greedy FILE [options]`

The options are:

- `--console_log`
  Write the log to the console.

- `--output=<FILE>`
  Path to store the resulting independent set.
  If not otherwise specified, a log file is produced.
  It is saved under the path, where the last file extension is stripped off the output string, and `.log` is appended.

- `--seed=<int>`
  Seed to use for the random number generator.

- `--time_limit=<double>`
  Time limit until the algorithm terminates.

- `--weight_source=<string>`
  Choose how the weights are assigned. Can be either: file (default), hybrid, uniform, geometric.

- `--log_file=<string>`
  The path where to save the log file to. The log file is in the JSON-format.

### simple_ml ###
Perform a greedy algorithm, prioritising based on a machine learning heuristic.

`simple_ml FILE [options]`

The options are:

- `--model=<string>` The path to the machine learning model to use.

- `--nn` Switch from the default random-forest heuristic to neural network based one.
  Use a corresponding model.

- `--console_log`
  Write the log to the console.

- `--output=<FILE>`
  Path to store the resulting independent set.
  If not otherwise specified, a log file is produced.
  It is saved under the path, where the last file extension is stripped off the output string, and `.log` is appended.

- `--seed=<int>`
  Seed to use for the random number generator.

- `--time_limit=<double>`
  Time limit until the algorithm terminates.

- `--weight_source=<string>`
  Choose how the weights are assigned. Can be either: file (default), hybrid, uniform, geometric.

- `--log_file=<string>`
  The path where to save the log file to. The log file is in the JSON-format.

### train_ml ###
Train a random forest based machine learning model.

`train_ml --model=<string> [--only_data]`

The string is the path where the resulting model should be saved to.
If it is a relative path, it is assumed to be relative to the models directory, 
which is under the project root.

`--only_data` Output only the feature matrices (for use in training the neural network).
Output location is controlled by `--model`

The program assumes there are two files in the current directory 
`train_graphs_list.txt` and `train_labels_list.txt`
These should contain paths to matching graphs, and label data.
Example:

`train_graphs_list.txt`:

A.graph

B.graph

C.graph

`train_labels_list.txt`:

A.graph.mis

B.graph.mis

C.graph.mis

### nn_training_e2e.sh ###
Trains a model using a neural network. 
`nn_training_e2e.sh MODEL_NAME [data_directory]`

Outputs a neural network model with the name `MODEL_NAME.nn_model.json` in the models directory.

### analyze ###
`analyze FILE` Check graph FILE for self-loops, duplicate edges, and for undirectedness.
`analyze model FILE` Output relative weights of features in the XGBoost model FILE.
`analyze mis FILE` Check whether set FILE is an independent set.
The file should contain one line for each node, consisting of 0 or 1 whether the node is element of the set.

### assign_weights ###
Assign weights to a graph.
`assign_weights FILE [options]`

`FILE` The graph to assign weights to.

The options are:

- `--output=FILE` Sets the resulting graph file.

- `--weight_source=(uniform|geometric|hybrid)` Sets the method of weight assignment.
  It uses either a uniform or a geometric probability distribution, or the modulus of its NodeID.

- `--max_weight=<int>` The maximum weight to assign a node.

- `--seed=<int>` The seed to use for random number generation.

### metis_to_dot ###
Convert a graph in the metis format to the dot format.

`metis_to_dot GRAPH_FILE [IS_FILE1] [IS_FILE2]` 

If only one independent set file is provided, then blue nodes represent nodes that are included in the IS, white not included.
If two IS files are provided, 
blue means included in both,
green means included in the first, but not in the second, 
red means included in the second, but not in the first
and white in neither.

