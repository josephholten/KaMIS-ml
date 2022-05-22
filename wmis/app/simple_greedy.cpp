//
// Created by joseph on 4/25/22.
//

#include "graph_access.h"
#include "graph_io.h"
#include "greedy_algorithm.h"
#include "ml_features.h"
#include "extern/algo_test/algo_log.h"
#include "util.h"

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

    if (argc < 2 || argc > 3) {
        std::cerr << "ERROR: Arguments: graph_filename [output_filename]" << std::endl;
        return 1;
    }

    algo_log::logger().instance(argv[1]);

    graph_access G;
    graph_io::readGraphWeighted(G, argv[1]);

    if (argc == 3)
        validate_path(argv[2]);

    greedy_algorithm<ht_heuristic> alg(G);

    algo_log::logger().start_timer();
    alg.run();
    algo_log::logger().end_timer();

    auto IS = alg.IS();
    auto weight = alg.getISWeight();

    check_IS(G, IS, weight);

    algo_log::logger().solution(weight);

    if (argc == 3) graph_io::writeVector(IS, argv[2]);

    std::cout << std::setw(2) << algo_log::logger() << std::endl;
}
