//
// Created by joseph on 3/18/22.
//
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <argtable3.h>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <random>

#include "timer.h"
#include "mis_log.h"
#include "graph_access.h"
#include "graph_io.h"
#include "mis_config.h"
#include "parse_parameters.h"
#include "branch_and_reduce_algorithm.h"
#include "graph_builder.h"

void assign_weights(graph_access& G, const MISConfig& mis_config) {
    constexpr NodeWeight MAX_WEIGHT = 200;

    if (mis_config.weight_source == MISConfig::Weight_Source::HYBRID) {
        forall_nodes(G, node) {
            G.setNodeWeight(node, (node + 1) % MAX_WEIGHT + 1);
        } endfor
    } else if (mis_config.weight_source == MISConfig::Weight_Source::UNIFORM) {
        std::default_random_engine generator(mis_config.seed);
        std::uniform_int_distribution<NodeWeight> distribution(1,MAX_WEIGHT);

        forall_nodes(G, node) {
            G.setNodeWeight(node, distribution(generator));
        } endfor
    } else if (mis_config.weight_source == MISConfig::Weight_Source::GEOMETRIC) {
        std::default_random_engine generator(mis_config.seed);
        std::binomial_distribution<int> distribution(MAX_WEIGHT / 2);

        forall_nodes(G, node) {
            G.setNodeWeight(node, distribution(generator));
        } endfor
    }
}

int main(int argc, char** argv) {
    MISConfig mis_config;
    std::string graph_filepath;

    // Parse the command line parameters;
    int ret_code = parse_parameters(argc, argv, mis_config, graph_filepath);
    if (ret_code) {
        return 0;
    }

    mis_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of('/') + 1);

    // Read the graph
    graph_access G;
    if (mis_config.graph_filename.substr(mis_config.graph_filename.find_last_of('.')) == ".edgelist") {
        graph_builder builder;
        builder.build_from_edgelist(G, graph_filepath);
    } else {
        graph_io::readGraphWeighted(G, graph_filepath);
    }

    assign_weights(G, mis_config);

    graph_io::writeGraphNodeWeighted(G, mis_config.output_filename);
}