//
// Created by joseph on 5/11/22.
//

#include <cassert>
#include <string>
#include <sstream>

#include "graph_access.h"
#include "graph_io.h"
#include "parse_parameters_synthetics.h"
#include "random_functions.h"
#include "synthetic_config.h"
#include "synthetic_graph_builder.h"


int main(int argc, char** argv) {
    synth_config config;
    std::string path;
    parse_parameters(argc, argv, config, path);
    validate_path(path);

    graph_access G;
    synthetic_graph_builder builder(config);
    builder.build_mixed(G);
    graph_io::writeGraphNodeWeighted(G, path);
}