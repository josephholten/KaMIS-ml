//
// Created by joseph on 3/21/22.
//

#include <bits/stdc++.h>
#include "graph_io.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Provide first the graph, then the IS" << std::endl;
        return 1;
    }

    graph_access G;
    graph_io::readGraphWeighted(G, argv[1]);

    std::vector<int> IS;
    graph_io::readVector(IS, argv[2]);

    NodeWeight weight = 0;
    forall_nodes(G, node) {
        if (IS[node])
            weight += G.getNodeWeight(node);
    } endfor

    std::cout << argv[1] << " " << weight << std::endl;
}
