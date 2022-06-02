//
// Created by joseph on 6/2/22.
//

#ifndef KAMIS_SYNTHETIC_GRAPH_BUILDER_H
#define KAMIS_SYNTHETIC_GRAPH_BUILDER_H

#include "graph_access.h"
#include <random>
#include <algorithm>
#include "graph_builder.h"
#include "synthetic_config.h"

class synthetic_graph_builder : public graph_builder {
public:
    explicit synthetic_graph_builder(const synth_config& _config) : graph_builder(), config {_config} {
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

        build(G);
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
    NodeID random_size() {
        std::uniform_int_distribution<NodeID> size_distribution(1, std::max(1ul,((config.size - nodes) / config.types.size())));
        return size_distribution(generator);
    };
};


#endif //KAMIS_SYNTHETIC_GRAPH_BUILDER_H
