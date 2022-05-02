//
// Created by joseph on 4/26/22.
//

#ifndef KAMIS_GREEDY_ALGORITHM_H
#define KAMIS_GREEDY_ALGORITHM_H

#include "priority_queue.h"
#include "graph_access.h"
#include "graph_io.h"
#include "fast_set.h"
#include "weighted_dynamic_graph.h"
#include "algo_log.h"

enum IS_status { included = 1, excluded = 0, not_set = -1};

template<typename priority_t, priority_direction direction>
class greedy_algorithm {
public:

    explicit greedy_algorithm(graph_access& G, std::function<priority_t(weighted_dynamic_graph, NodeID)> _heuristic, algo_log _logger)
            : graph(G), status(G.number_of_nodes(), IS_status::not_set), queue(G.number_of_nodes()), changed(G.number_of_nodes()),
              changed_set(G.number_of_nodes()), heuristic(_heuristic), logger(_logger)
    {
        std::iota(changed.begin(), changed.end(), 0);
    }

    void run() {
        logger.start_timer();

        while(!queue.empty()) {
            // update
            for (NodeID node : changed)
                queue.set(node, heuristic(graph, node));

            changed.clear();
            changed_set.clear();

            if (size_t top = queue.pop(); status[top] == IS_status::not_set)
                set_status(top, IS_status::included);
        }

        logger.end_timer();
    }

    const std::vector<IS_status>& IS() {
        return status;
    }

    NodeWeight getISWeight() {
        return is_weight;
    }

private:
    weighted_dynamic_graph graph;
    std::vector<IS_status> status;
    priority_queue<priority_t, direction> queue;
    std::vector<NodeID> changed;
    fast_set changed_set;
    NodeWeight is_weight = 0;
    algo_log logger;

    std::function<priority_t(weighted_dynamic_graph, NodeID)> heuristic;

    void set_status(NodeID node, IS_status is_status) {
        status[node] = is_status;
        graph.hide_node(node);

        if (is_status == IS_status::included) {
            is_weight += graph.getNodeWeight(node);

            for (auto neighbor : graph[node]) {
                status[neighbor] = IS_status::excluded;
                graph.hide_node(neighbor);

                for (auto neighbors_neighbor : graph[neighbor]) {
                    if (neighbors_neighbor != node && changed_set.add(neighbors_neighbor))
                        changed.push_back(neighbors_neighbor);
                }
            }
        }
    }
};

#endif //KAMIS_GREEDY_ALGORITHM_H