//
// Created by joseph on 4/25/22.
//

#include "graph_access.h"
#include "graph_io.h"
#include "greedy_algorithm.h"

NodeWeight ht(const weighted_dynamic_graph& graph, NodeID node) {
    NodeWeight w = 0;
    for (auto neighbor : graph[node])
        w += graph.getNodeWeight(neighbor);
    w -= graph.getNodeWeight(node);
    return w;
}

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

    if (argc < 2 || argc > 3) {
        std::cerr << "ERROR: Arguments: graph_filename [output_filename]" << std::endl;
        return 1;
    }

    graph_access G;
    graph_io::readGraphWeighted(G, argv[1]);

    greedy_algorithm<NodeWeight, priority_direction::MIN> alg(G, ht);
    alg.run();

    auto IS = alg.IS();
    auto weight = alg.getISWeight();

    check_IS(G, IS, weight);

    std::cout << "MIS_weight " << weight << std::endl;

    if (argc == 3)
        graph_io::writeVector(IS, argv[2]);
}
