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
        if (config.types[0] == graph_family::ba || config.types[0] == graph_family::gnp) {
            add(config.size, config.types[0]);
            build(G);
            return;
        }

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
        } else if (family == graph_family::gnp) {
            std::uniform_real_distribution<> random(0.0,1.0);
            for (size_t i = 0; i < n; ++i)
                add_node();

            for (size_t source = 0; source < n; ++source)
                for (size_t target = source + 1; target < n; ++target)
                    if (random(generator) < config.p)
                        add_edge(source, target);
        } else if (family == graph_family::ba) {
            std::uniform_real_distribution<> random(0.0,1.0);

            for (size_t i = 0; i < n; ++i) 
                add_node();

            // initial complete graph
            for (size_t source = 0; source < config.m; ++source) 
                for (size_t target = source + 1; target < config.m; ++target)
                    add_edge(source, target);

            std::cout << adj.size() << std::endl;

            // preferential attachment
            for (size_t source = config.m; source < n; ++source)
                for (size_t target = 0; target < source; ++target)
                    if (random(generator) < (double) degree(target) / (2*edges)) {
                        add_edge(source, target);
                    }
        }
        assert(nodes == (start_nodes + n));
    };

private:
    const synth_config& config;
    size_t random_size() {
        std::uniform_int_distribution<NodeID> size_distribution(1, std::max(1ul,((config.size - nodes) / config.types.size())));
        return size_distribution(generator);
    };

    size_t degree(NodeID node) {
        return adj[node].size();
    }
};


#endif //KAMIS_SYNTHETIC_GRAPH_BUILDER_H
