//
// Created by joseph on 5/11/22.
//

#include <cassert>
#include <string>
#include <sstream>

#include "graph_access.h"
#include "graph_io.h"
#include "parse_parameters_synthetics.h"
#include "random_functions.h"
#include "util.h"

class graph_builder {
public:
    graph_builder(const synth_config& _config) : config {_config} {
        generator.seed(config.seed);
        weight_distribution = std::uniform_int_distribution<NodeWeight>(1, config.max_weight);

        adj.resize(config.size);
        weights.resize(config.size);
    }

    void build_mixed(graph_access& G) {
        std::uniform_int_distribution<size_t> random_family_idx(0, config.types.size()-1);
        while (nodes < config.size) {
            add(random_size(), config.types[random_family_idx(generator)]);
        }
        for (auto& neighborhood : adj)
            std::sort(neighborhood.begin(), neighborhood.end());

        std::vector<EdgeID> start;
        std::vector<NodeID> edge_arr;
        for (auto neighborhood : adj) {
            start.push_back(edge_arr.size());
            std::copy(neighborhood.begin(), neighborhood.end(), std::back_inserter(edge_arr));
        }
        start.push_back(edge_arr.size());
        G.build_from_metis(start, edge_arr, weights);
    };

    void add(NodeID n, graph_family family) {
        auto start_nodes = nodes;
        if (n + nodes > adj.size()) {
            adj.resize(n + nodes);
            weights.resize(n + nodes);
        }

        if (family == graph_family::path) {
            for (size_t i = 0; i < n - 1; ++i) {
                auto node = add_node();
                add_edge(node, node + 1);
            }
            add_node();
        } else if (family == graph_family::cycle) {
            auto first_node_of_cycle = current_node() + 1;
            add(n, graph_family::path);
            if (n > 2)
                add_edge(current_node(), first_node_of_cycle);
        } else if (family == graph_family::star) {
            auto center = add_node();
            for (size_t i = 1; i < n; ++i) {
                add_edge(center, add_node());
            }
        } else if (family == graph_family::tree) {
            auto parent = add_node();  // root

            NodeID remaining_nodes = n-1;
            while (remaining_nodes > 0) {
                std::uniform_int_distribution<EdgeID> random_degree(1, remaining_nodes);
                auto degree = random_degree(generator);

                for (NodeID i = 0; i < degree; ++i) {
                    add_edge(parent, add_node());
                }

                remaining_nodes -= degree;
                parent++;
            }
        }
        assert(nodes == (start_nodes + n));
    };

private:
    const synth_config& config;
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
        assert(source <= adj.size() && target <= adj.size());
        adj[source].push_back(target);
        adj[target].push_back(source);
        edges++;
    }

    NodeWeight random_weight() {
        return weight_distribution(generator);
    };

    NodeID random_size() {
        std::uniform_int_distribution<NodeID> size_distribution(1, std::max(1ul,((config.size - nodes) / config.types.size())));
        return size_distribution(generator);
    };
};

int main(int argc, char** argv) {
    synth_config config;
    std::string path;
    parse_parameters(argc, argv, config, path);
    validate_path(path);

    graph_access G;
    graph_builder builder(config);
    builder.build_mixed(G);
    graph_io::writeGraphNodeWeighted(G, path);
}