/**
 * reduction_evomis.cpp
 * Purpose: Main program for the evolutionary algorithm.
 *
 *****************************************************************************/

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
#include "parse_parameters_ml.h"
#include "branch_and_reduce_algorithm.h"
#include "extern/algo_test/algo_log.h"
#include "util.h"

bool is_IS(graph_access& G) {
        forall_nodes(G, node) {
                if (G.getPartitionIndex(node) == 1) {
                        forall_out_edges(G, edge, node) {
                                NodeID neighbor = G.getEdgeTarget(edge);
                                if (G.getPartitionIndex(neighbor) == 1) {
                                        return false;
                                }
                        } endfor
                }
        } endfor

        return true;
}

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

int main(int argn, char **argv) {
    algo_log::logger().set_name("simple_ml");

    MISConfig mis_config;
    std::string graph_filepath;

    // Parse the command line parameters;
    int ret_code = parse_parameters_ml(argn, argv, mis_config, graph_filepath);
    if (ret_code) {
        return 0;
    }
    if (mis_config.write_graph)
        validate_path(mis_config.output_filename);

    // always choose the machine learning reductions
    mis_config.reduction_style = MISConfig::SIMPLE_ML;

    mis_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of('/') + 1);

    // Read the graph
    graph_access G;
    std::string comments;
    graph_io::readGraphWeighted(G, graph_filepath, comments);
    assign_weights(G, mis_config);


    algo_log::logger().start_timer();
    branch_and_reduce_algorithm reducer(G, mis_config);
    reducer.run_branch_reduce();
    algo_log::logger().end_timer();

    algo_log::logger().solution(reducer.get_current_is_weight());

    reducer.apply_branch_reduce_solution(G);

    if (!is_IS(G)) {
        std::cerr << "ERROR: graph after inverse reduction is not independent" << std::endl;
        exit(1);
    } else {
        NodeWeight is_weight = 0;

        forall_nodes(G, node) {
            if (G.getPartitionIndex(node) == 1)
                is_weight += G.getNodeWeight(node);
        } endfor

        if (algo_log::logger()["solution"] != is_weight) {
            std::cerr << "ERROR: failed MIS weight check!" << std::endl;
            return 1;
        }
    }

    if (mis_config.write_graph) graph_io::writeIndependentSet(G, mis_config.output_filename);

    algo_log::logger().write(mis_config.log_file);
    std::cout << std::setw(2) << algo_log::logger() << std::endl;

    return 0;
}
