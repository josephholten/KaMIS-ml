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
    mis_log::instance()->restart_total_timer();

    MISConfig mis_config;
    std::string graph_filepath;

    // Parse the command line parameters;
    int ret_code = parse_parameters_ml(argn, argv, mis_config, graph_filepath);
    if (ret_code) {
        return 0;
    }

    mis_config.reduction_style = MISConfig::GREEDY;

    mis_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of('/') + 1);
    if (mis_config.console_log) {
        std::cout << "graph " << mis_config.graph_filename << std::endl;
        std::cout << "config " << "ls_rounds " << mis_config.ls_rounds << ", "
                  << "ls_time " << mis_config.ls_time << ", "
                  << " ml_pruning " << mis_config.ml_pruning
                  << std::endl;
    }
    mis_log::instance()->set_config(mis_config);

    // Read the graph
    graph_access G;
    std::string comments;
    graph_io::readGraphWeighted(G, graph_filepath, comments);
    assign_weights(G, mis_config);

    mis_log::instance()->set_graph(G);
    algo_log::logger().instance(graph_filepath);

    //std::cout << "%nodes " << G.number_of_nodes() << std::endl;
    //std::cout << "%edges " << G.number_of_edges() << std::endl;

    auto start = std::chrono::system_clock::now();
    algo_log::logger().start_timer();

    branch_and_reduce_algorithm reducer(G, mis_config);
    reducer.run_branch_reduce();
    NodeWeight MWIS_weight = reducer.get_current_is_weight();

    algo_log::logger().end_timer();
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<float> branch_reduce_time = end - start;
    std::cout << "time " << branch_reduce_time.count() << "\n";
    std::cout << "MIS_weight " << MWIS_weight << "\n";

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


        if (MWIS_weight != is_weight) {
            std::cout << "IS weight wrong, double check! " << MWIS_weight << is_weight << std::endl;
        }
        algo_log::logger().solution(is_weight);
    }

    algo_log::logger().write(mis_config.log_file);
    if (mis_config.write_graph) graph_io::writeIndependentSet(G, mis_config.output_filename);

    std::cout << std::endl;

    return 0;
}
