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

void assign_weights(graph_access& G, const synth_config& config, std::default_random_engine& generator) {
    NodeWeight MAX_WEIGHT = std::min(200u, config.max_weight);

    if (config.source == synth_config::weight_source::HYBRID) {
        forall_nodes(G, node) {
            G.setNodeWeight(node, (node + 1) % MAX_WEIGHT + 1);
        } endfor
    } else if (config.source == synth_config::weight_source::UNIFORM) {
        std::uniform_int_distribution<NodeWeight> distribution(1,MAX_WEIGHT);

        forall_nodes(G, node) {
            G.setNodeWeight(node, distribution(generator));
        } endfor
    } else if (config.source == synth_config::weight_source::GEOMETRIC) {
        MAX_WEIGHT = std::max(2u,MAX_WEIGHT);
        std::binomial_distribution<NodeWeight> distribution(MAX_WEIGHT / 2);

        forall_nodes(G, node) {
            G.setNodeWeight(node, distribution(generator));
        } endfor
    }
}

void gen_graph(graph_access& G, graph_family family, const synth_config& config, std::default_random_engine& generator) {
    size_t n = random_functions::nextInt(config.max_size, config.max_size);
    size_t m;

    if (family == graph_family::path || family == graph_family::tree || family == graph_family::star ) {
        m = n - 1;
    } else if (family == graph_family::cycle) {
        m = n;
    }

    // forward & back edges
    m *= 2;

    std::vector<NodeID> start(n+1, 0);
    std::vector<EdgeID> edges(m, 0);

    if (family == graph_family::path) {
        start[1] = 1;
        for (size_t i = 2; i < n; ++i)
            start[i] = start[i - 1] + 2;
        start[n] = m;

        edges[0] = 1;
        for (size_t i = 1; i < n - 1; ++i) {
            edges[2*i-1] = i - 1;
            edges[2*i  ] = i + 1;
        }
        edges[m-1] = n-2;
    } else if (family == graph_family::cycle ) {
        for (size_t i = 1; i < n; ++i)
            start[i] = start[i - 1] + 2;
        start[n] = m;

        for (size_t i = 0; i < n; ++i) {
            edges[2*i    ] = (i - 1) % n;
            edges[2*i + 1] = (i + 1) % n;
        }
    } else if (family == graph_family::star) {
        // node 0 is center node, so node 1 starts at deg(0)
        start[1] = n-1;
        for (size_t i = 2; i < n; ++i)
            start[i] = start[i-1] + 1;
        start[n] = m;

        // edges from center
        for (size_t i = 0; i < n; ++i)
            edges[i] = i + 1;
        // edges to center
        for (size_t i = n; i < m; ++i)
            edges[i] = 0;
    } else if (family == graph_family::tree) {
        auto remaining = m;
        size_t node = 0;

        std::vector<std::vector<NodeID>> adj(1);

        while (remaining > 0) {
            std::uniform_int_distribution<NodeWeight> distribution(1,remaining);
            auto degree = distribution(generator);
            remaining -= 2*degree;

            // adj contains vectors for each created node, neighbor is next
            auto neighbor = adj.size();

            // degree new nodes
            adj.resize(adj.size() + degree);

            for (size_t d = 0; d < degree; ++d) {
                adj[node].push_back(neighbor + d);
                adj[neighbor + d].push_back(node);
            }

            ++node;
        }

        start.clear();
        edges.clear();
        for (auto neighborhood : adj) {
            start.push_back(edges.size());
            std::copy(neighborhood.begin(), neighborhood.end(), std::back_inserter(edges));
        }
        start.push_back(edges.size());
    }

    G.build_from_metis(start, edges);
    assign_weights(G, config, generator);
}

int main(int argc, char** argv) {
    synth_config config;
    std::string path;
    parse_parameters(argc, argv, config, path);

    {
        std::ofstream file(path + "test");
        assert(file.is_open() && "invalid path");
        std::remove((path + "test").c_str());
    }

    graph_access G;
    std::default_random_engine generator(config.seed);

    for (auto type : config.types) {
        for (int i = 0; i < config.instances; ++i) {
            gen_graph(G, type, config, generator);
            std::stringstream ss;
            ss << path << "/" << family_to_str.at(type) << i << ".graph";
            std::ofstream file(ss.str());
            config.serialize(file);
            graph_io::writeGraphNodeWeighted(G, file);
        }
    }
}