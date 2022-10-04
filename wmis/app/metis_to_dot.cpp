//
// Created by joseph on 5/7/22.
//
#include "graph_io.h"
#include "graph_access.h"

int main(int argc, char** argv) {
    assert(argc >= 2 && argc <= 4 && "Args: graph_filename [is1] [is2]");
    graph_access G;
    graph_io::readGraphWeighted(G, argv[1]);

    // reading IS's and getting max weight
    std::vector<int> is_1(G.number_of_nodes()), is_2(G.number_of_nodes());

    if (argc >= 3)
        graph_io::readVector(is_1, argv[2]);
    if (argc == 4)
        graph_io::readVector(is_2, argv[3]);

    double avg_weight = 0;
    forall_nodes(G, node) {
                avg_weight = G.getNodeWeight(node);
    } endfor
    avg_weight /= G.number_of_nodes();


    std::cout << "strict graph {" << std::endl;

    // nodes, their fill colors (IS status) and their label (weight)
    std::cout << "  {" << std::endl;
    std::cout << "    node [style=filled]" << std::endl;
    forall_nodes(G, node) {
        std::cout << "    " << node << " [";
        if (argc >= 3) {
            std::string color;

            // only one IS, so color means included or not included
            if (argc == 3) {
                 color = is_1[node] ? "lightblue" : "white";
            }
            // two separate IS, so color indicates inclusion in (which) one, none or both
            else if (argc == 4) {
                if (is_1[node] == 1 && is_2[node] == 1) {
                    color = "lightblue";
                }
                if (is_1[node] == 1 && is_2[node] == 0) {
                    color = "lightgreen";
                }
                if (is_1[node] == 0 && is_2[node] == 1) {
                    color = "red";
                }
                if (is_1[node] == 0 && is_2[node] == 0) {
                    color = "white";
                }
            }
            std::cout << "fillcolor=" << color << ",";
        }

        std::cout << "label=" << G.getNodeWeight(node) << ",";
        std::cout << "fixedsize=true,";
        double size = exp(0.2 * (G.getNodeWeight(node) - avg_weight));
        std::cout << "height=" << size << ",width=" << size << ",";

        std::cout << "]" << std::endl;
    } endfor

    std::cout << "  }" << std::endl;

    // edges
    forall_nodes(G, node) {
        std::cout << "  " << node << " -- {";
        forall_out_edges(G, edge, node) {
            std::cout << G.getEdgeTarget(edge) << " ";
        } endfor
        std::cout << "}" << std::endl;
    } endfor

    std::cout << "}" << std::endl;
}