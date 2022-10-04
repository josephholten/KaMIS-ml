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
#include "extern/algo_test/algo_log.h"

enum IS_status { included = 1, excluded = 0, not_set = -1};

// Implementation of a simple greedy algorithm, not using any reductions
template<typename Heuristic>
class greedy_algorithm {
public:
    using priority_t = typename Heuristic::priority_t;
    static const priority_direction direction = Heuristic::direction;

    explicit greedy_algorithm(graph_access& G)
            : graph(G), status(G.number_of_nodes(), IS_status::not_set), queue(G.number_of_nodes()), changed(G.number_of_nodes()),
              changed_set(G.number_of_nodes()), heuristic()
    {
        std::iota(changed.begin(), changed.end(), 0);
    }

    void run() {
        while(!queue.empty()) {
            // update priority values for changed nodes
            heuristic.updateMany(graph, changed, [&](NodeID node, priority_t new_priority){ queue.set(node, new_priority); });

            changed.clear();
            changed_set.clear();

            // if the top node is not yet included, include it, otherwise continue
            if (size_t top = queue.pop(); status[top] == IS_status::not_set)
                set_status(top, IS_status::included);
        }
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
    Heuristic heuristic;

    void set_status(NodeID node, IS_status is_status) {
        status[node] = is_status;
        graph.hide_node(node);

        if (is_status == IS_status::included) {
            is_weight += graph.getNodeWeight(node);

            // If node is included, exclude the neighbours to maintain a feasible solution
            for (auto neighbor : graph[node]) {
                status[neighbor] = IS_status::excluded;
                graph.hide_node(neighbor);

                // By hiding the neighbor, its neighbors will change
                for (auto neighbors_neighbor : graph[neighbor]) {
                    if (neighbors_neighbor != node && changed_set.add(neighbors_neighbor))
                        changed.push_back(neighbors_neighbor);
                }
            }
        }
    }
};

#endif //KAMIS_GREEDY_ALGORITHM_H
