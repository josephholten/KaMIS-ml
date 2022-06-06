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
    graph_builder() {
        weight_distribution = std::uniform_int_distribution<NodeWeight>(1, 10);
    }

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
            adj[source-1].push_back(target-1);
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
        for (NodeID node = 0; node < adj.size(); ++node) {
            auto neighborhood = adj[node];
            start.push_back(edge_arr.size());
            std::sort(neighborhood.begin(), neighborhood.end());
            for (long long i = 0; i < (long long) neighborhood.size() - 1; ++i) {
                // if neither duplicate nor self loop, copy
                if (!(neighborhood[i] == neighborhood[i+1] || neighborhood[i] == node))
                    edge_arr.push_back(neighborhood[i]);
            }
            if (neighborhood.size() >= 2 && neighborhood[neighborhood.size()-1] != neighborhood[neighborhood.size()-2])
                edge_arr.push_back(neighborhood[neighborhood.size()-1]);
        }
        start.push_back(edge_arr.size());
        G.build_from_metis(start, edge_arr, weights);
    }
};

#endif //KAMIS_GRAPH_BUILDER_H
