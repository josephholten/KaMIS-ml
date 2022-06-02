//
// Created by joseph on 6/2/22.
//

#ifndef KAMIS_GRAPH_BUILDER_H
#define KAMIS_GRAPH_BUILDER_H

#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "definitions.h"
#include "graph_access.h"

class graph_builder {
public:
    void build_from_edgelist(graph_access& G, const std::string& filepath) {
        std::ifstream file(filepath);
        std::string line;
        std::string type;
        getline(file, line);
        std::stringstream ss(line);
        size_t n, m;
        ss >> type >> n >> m;
        if (type != "p") {
            std::cerr << "invalid edgelist format, must start with 'p n m'" << std::endl;
            std::exit(1);
        }

        adj.resize(n);
        weights.resize(n);

        while(getline(file, line)) {
            std::stringstream ss(line);
            NodeID source, target;
            ss >> type >> source >> target;
            add_edge(source-1, target-1);
        }

        for (NodeID node = 0; node < n; ++node)
            add_node();

        build(G);
    }

protected:
    std::mt19937 generator;
    std::uniform_int_distribution<NodeWeight> weight_distribution;

    NodeID nodes = 0;
    NodeID edges = 0;
    std::vector<std::vector<NodeID>> adj;
    std::vector<NodeWeight> weights;

    NodeID current_node() {
        return nodes - 1;
    }

    NodeID add_node() {
        assert(nodes < adj.size());
        weights[nodes] = random_weight();
        return nodes++;
    }

    void add_edge(NodeID source, NodeID target) {
        assert(source < adj.size() && target < adj.size());
        adj[source].push_back(target);
        adj[target].push_back(source);
        edges++;
    }

    NodeWeight random_weight() {
        return weight_distribution(generator);
    };

    void build(graph_access& G) {
        std::vector<EdgeID> start;
        std::vector<NodeID> edge_arr;
        for (auto neighborhood : adj) {
            start.push_back(edge_arr.size());
            std::copy(neighborhood.begin(), neighborhood.end(), std::back_inserter(edge_arr));
        }
        start.push_back(edge_arr.size());
        G.build_from_metis(start, edge_arr, weights);
    }
};

#endif //KAMIS_GRAPH_BUILDER_H
