//
// Created by joseph on 4/25/22.
//

#include "graph_access.h"
#include "graph_io.h"
#include "greedy_algorithm.h"
#include "ml_features.h"
#include "extern/algo_test/algo_log.h"
#include "util.h"
#include "parse_parameters.h"

class ht_heuristic {
public:
    typedef NodeWeight priority_t;
    static const priority_direction direction = priority_direction::MIN;

    template<typename Function>
    void updateMany(const weighted_dynamic_graph& graph, const std::vector<NodeID>& nodes, Function update) {
        for (auto node : nodes)
            update(node, eval(graph, node));
    }

    static NodeWeight eval(weighted_dynamic_graph graph, NodeID node) {
        NodeWeight w = 0;
        for (auto neighbor : graph[node])
            w += graph.getNodeWeight(neighbor);
        w -= graph.getNodeWeight(node);
        return w;
    }
};

void check_IS(graph_access& G, std::vector<IS_status> IS, NodeWeight weight) {
    bool res = true;
    NodeWeight true_weight = 0;
    forall_nodes(G, node) {
        if (IS[node] == IS_status::included) {
            true_weight += G.getNodeWeight(node);
            forall_out_edges(G, edge, node){
                assert(IS[G.getEdgeTarget(edge)] == IS_status::excluded);
            } endfor
        }
        assert(IS[node] != IS_status::not_set);
    } endfor

    assert(weight == true_weight);
}

int main(int argc, char** argv) {
    // test_min_priority_queue();
    algo_log::logger().set_name("simple_greedy");

    MISConfig mis_config;
    std::string graph_filepath;
    if (int ret_code = parse_parameters(argc, argv, mis_config, graph_filepath); ret_code)
        return ret_code;

    algo_log::logger().instance(graph_filepath);

    graph_access G;
    graph_io::readGraphWeighted(G, graph_filepath);

    if (argc == 3)
        validate_path(argv[2]);
    if (argc == 4)
        validate_path(argv[4]);

    greedy_algorithm<ht_heuristic> alg(G);

    algo_log::logger().start_timer();
    alg.run();
    algo_log::logger().end_timer();

    const auto& IS = alg.IS();
    auto weight = alg.getISWeight();

    check_IS(G, IS, weight);

    algo_log::logger().solution(weight);

    if (mis_config.write_graph) graph_io::writeVector(IS, mis_config.output_filename);

    algo_log::logger().write(mis_config.log_file);
    return 0;
}
