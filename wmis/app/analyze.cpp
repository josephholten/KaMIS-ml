//
// Created by joseph on 2/8/22.
//

#include <graph_access.h>
#include <graph_io.h>

bool strCompare(const std::string & str1, const std::string & str2) {
    return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2){ return std::toupper(c1) == std::toupper(c2); });
}

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

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Must provide a method and at least one file." << std::endl;
        return 1;
    }

    // analyze model
    if (strCompare(argv[1],"model")) {
        return 1;
    }

    // check whether set is independent
    if (strCompare(argv[1],"mis")) {
        if (argc != 5) {
            std::cerr << "For mis analysis you must first provide a graph, then a IS and it's type" << std::endl;
            return 1;
        }

        std::cout << "Reading graph: " << argv[2] << std::endl;
        graph_access G;
        graph_io::readGraphWeighted(G, argv[2]);


        std::cout << "Reading IS: " << argv[3] << std::endl;
        std::ifstream IS_file(argv[3]);

        if (strCompare(argv[4], "kamis")) {
            std::string IS_str(std::istreambuf_iterator<char>{IS_file}, {});
            for (std::size_t index = 0; index < IS_str.size(); index += 2) {
                G.setPartitionIndex(index / 2, IS_str[index] - '0');
            }
        }

        if (strCompare(argv[4], "amazon")) {
            NodeID node;
            while (IS_file >> node) {
                G.setPartitionIndex(node, 1);
            }
        }


        std::cout << "Checkingc IS: " << argv[3] << std::endl;
        bool IS = is_IS(G);

        std::cout << "Independent set check: " << (IS ? "passed" : "failed") << std::endl;
        return IS ? 0 : 1;
    }

    std::cerr << "Must specify a valid method" << std::endl;

    return 1;

}
