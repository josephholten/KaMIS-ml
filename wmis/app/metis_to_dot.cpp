//
// Created by joseph on 5/7/22.
//
#include "graph_io.h"
#include "graph_access.h"

int main(int argc, char** argv) {
    assert(argc <= 4 && "Args: graph_filename [is1] [is2]");
    graph_access G;
    graph_io::readGraphWeighted(G, argv[1]);

    std::cout << "strict graph {" << std::endl;

    if (argc == 3) {
        std::vector<int> is(G.number_of_nodes());
        graph_io::readVector(is, argv[2]);

        std::cout << "  {" << std::endl;
        std::cout << "    node [style=filled]" << std::endl;
        forall_nodes(G, node) {
            std::cout << "    " << node << " [fillcolor=" << (is[node] ? "lightblue]" : "white]") << std::endl;
        } endfor
        std::cout << "  }" << std::endl;
    } else if (argc == 4) {
        std::vector<int> is_1(G.number_of_nodes());
        graph_io::readVector(is_1, argv[2]);

        std::vector<int> is_2(G.number_of_nodes());
        graph_io::readVector(is_2, argv[3]);

        std::cout << "  {" << std::endl;
        std::cout << "    node [style=filled]" << std::endl;

        NodeWeight max_weight = 0;
        forall_nodes(G, node) {
            max_weight = std::max(max_weight, G.getNodeWeight(node));
        } endfor

        forall_nodes(G, node) {
            if (is_1[node] == 1 && is_2[node] == 1) {
                std::cout << "    " << node << " [fillcolor=lightblue";
            }
            if (is_1[node] == 1 && is_2[node] == 0) {
                std::cout << "    " << node << " [fillcolor=lightgreen";
            }
            if (is_1[node] == 0 && is_2[node] == 1) {
                std::cout << "    " << node << " [fillcolor=red";
            }
            if (is_1[node] == 0 && is_2[node] == 0) {
                std::cout << "    " << node << " [fillcolor=white";
            }
            std::cout << ",fixedsize=true";
            std::cout << ",height=" << (double) G.getNodeWeight(node) / max_weight;
            std::cout << ",width="  << (double) G.getNodeWeight(node) / max_weight;
            std::cout << "]" << std::endl;
        } endfor
        std::cout << "  }" << std::endl;
    }

    forall_nodes(G, node) {
        std::cout << "  " << node << " -- {";
        forall_out_edges(G, edge, node) {
            std::cout << G.getEdgeTarget(edge) << " ";
        } endfor
        std::cout << "}" << std::endl;
    } endfor

    std::cout << "}" << std::endl;
}