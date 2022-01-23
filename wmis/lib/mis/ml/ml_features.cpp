//
// Created by joseph on 10/27/21.
//

#include "ml_features.h"

#include <random>
#include <unordered_set>

#include "mis_config.h"
#include "configuration_mis.h"
#include "timer.h"
#include "graph_io.h"
#include "weighted_ls.h"
#include "stat.h"

// constructors
// for training (many graphs with labels)
ml_features::ml_features(const MISConfig& config)
    : feature_matrix(FEATURE_NUM), has_labels {true}
{
    configuration_mis cfg;
    cfg.standard(mis_config);
    mis_config.console_log = config.console_log;
    mis_config.ml_pruning = config.ml_pruning;
    mis_config.time_limit = config.ls_time;
    mis_config.ls_rounds = config.ls_rounds;
}

// for reducing (single graph without labels)
ml_features::ml_features(const MISConfig& config, graph_access &G)
    : feature_matrix(FEATURE_NUM), has_labels {false}
{
    configuration_mis cfg;
    cfg.standard(mis_config);
    mis_config.console_log = config.console_log;
    mis_config.ls_updates = config.ls_updates;
    mis_config.ml_pruning = config.ml_pruning;
    mis_config.time_limit = config.ls_time;
    mis_config.ls_rounds = config.ls_rounds;
    reserveNodes(G.number_of_nodes());
    fillGraph(G);
}

ml_features::~ml_features() {
    XGDMatrixFree(dmat);
}

// getters
int ml_features::getNumberOfFeatures() {
    return FEATURE_NUM;
}

void ml_features::fromPaths(const std::vector<std::string> &all_graph_paths, const std::vector<std::string>& all_label_paths) {
    assert((all_graph_paths.size() == all_label_paths.size()) && "Error: provide same number of graph and label paths\n");

    std::vector<std::string> graph_paths;
    std::vector<std::string> label_paths;
    for (int i = 0; i < all_graph_paths.size(); ++i) {
        if (graph_io::readNumberOfNodes(all_graph_paths[i]) > 0) {
            graph_paths.push_back(all_graph_paths[i]);
            label_paths.push_back(all_label_paths[i]);
        }
    }
    std::cout << "LOG: ml-features: of " << all_graph_paths.size() << " " << graph_paths.size() << " were non empty\n";

    // calculate the node offsets of each graph
    // produces a mapping of index in the list of paths (provided by the user) and the start position in the feature_matrix and label_data
    std::vector<NodeID> offsets(graph_paths.size()+1,0);
    for (int i = 0; i < graph_paths.size(); i++) {
        offsets[i+1] = offsets[i] + graph_io::readNumberOfNodes(graph_paths[i]);
    }

    // last offset is total number of nodes
    reserveNodes(offsets[graph_paths.size()]);

    // #pragma omp parallel for default(none) shared(graph_paths, label_paths, offsets, std::cout)
    for (int i = 0; i < graph_paths.size(); ++i) {
        // std::cout << "Thread " << omp_get_thread_num() << std::endl;
        std::cout << "LOG: ml-features: graph " << graph_paths[i] << " (" << (float) i / graph_paths.size() * 100 << "%)\n";
        graph_access G;
        graph_io::readGraphWeighted(G, graph_paths[i]);
        std::vector<float> labels(G.number_of_nodes(), 0);
        graph_io::readVector<float>(labels.begin(), offsets[i+1]-offsets[i], label_paths[i]);

        fillGraph(G, labels, offsets[i]);
    }
}

// init
void ml_features::reserveNodes(NodeID n) {
    feature_matrix.addRows(n);
    label_data.resize(label_data.size() + n);
}

void ml_features::fillGraph(graph_access& G) {
    if (has_labels) {
        std::cerr << "Error: feature matrix was constructed for labels, so provide the labels for the graph.\n";
        exit(1);
    }
    features(G);
}

void ml_features::fillGraph(graph_access& G, std::vector<float>& labels, NodeID offset) {
    if (!has_labels) {
        std::cerr << "Error: feature matrix was constructed not for labels, so the labels will be discarded.\n";
    }
    features(G);
    std::copy(labels.begin(), labels.end(), label_data.begin() + offset);
}



template<ml_features::feature f>
float& ml_features::getFeature(NodeID node) {
    return feature_matrix[node + current_size][f];
}

void ml_features::features(graph_access& G) {
    // timer t;

    NodeWeight total_weight = 0;
    forall_nodes(G, node) {
        total_weight += G.getNodeWeight(node);
    } endfor

    // greedy node coloring
    std::vector<int> node_coloring(G.number_of_nodes());
    std::vector<bool> available(G.number_of_nodes(), true);

    forall_nodes(G, node) {
        std::fill(available.begin(), available.end(), true);
        forall_out_edges(G, edge, node) {
            available[node_coloring[G.getEdgeTarget(edge)]] = false;
        } endfor
        node_coloring[node] = (int) (std::find_if(available.begin(), available.end(), [](bool x){ return x; }) - available.begin());
    } endfor
    int greedy_chromatic_number = *std::max_element(node_coloring.begin(), node_coloring.end()) + 1;
    std::vector<bool> used_colors(greedy_chromatic_number);

    // local search
    std::vector<int> ls_signal(G.number_of_nodes(), 0);
    std::random_device rd;
    // int seed = rd();
    // mis_config.seed = seed;

    // TODO: log correctly
    for (int round = 0; round < mis_config.ls_rounds; ++round) {
        // TODO: only reduce the graph once, then perform the LS rounds with different seeds
        mis_config.seed = (int) rd();
        std::cout << "ls_round " << round << std::endl;
        weighted_ls ls(mis_config, G);
        ls.run_ils();
        forall_nodes(G, node) {
            ls_signal[node] += (int) G.getPartitionIndex(node);
        } endfor
    }

    if (mis_config.console_log) {
        int count = 0;
        double mean = 0;
        double std_dev = 0;
        for (const auto& val : ls_signal) {
            if (val != 0) {
                mean += val;
                ++count;
            }
        }
        mean /= count;
        for (const auto& val : ls_signal) {
            if (val != 0) {
                std_dev += (mean - val) * (mean - val);
            }
        }
        std_dev /= count;

        std::cout << "ls_std_dev " << std_dev << std::endl;
    }
    // loop variales
    EdgeID local_edges = 0;
    std::unordered_set<NodeID> neighbors {};      // don't know how to do faster? maybe using bitset from boost?
    neighbors.reserve(G.getMaxDegree()+1);
    float avg_lcc = 0;
    float avg_wdeg = 0;

    forall_nodes(G, node){
        // num of nodes/ edges, deg
        getFeature<NODES>(node) = (float) G.number_of_nodes();
        getFeature<EDGES>(node) = (float) G.number_of_edges();
        getFeature<DEG>(node) = (float) G.getNodeDegree(node);

        // lcc
        neighbors.clear();
        forall_out_edges(G, edge, node) {
            neighbors.insert(G.getEdgeTarget(edge));
        } endfor
        for (auto& neighbor : neighbors) {
            forall_out_edges(G, neighbor_edge, neighbor) {
                if (neighbors.find(G.getEdgeTarget(neighbor_edge)) != neighbors.end())
                    ++local_edges;
            } endfor
        }

        if (G.getNodeDegree(node) > 1)  // catch divide by zero
            getFeature<LCC>(node) = ((float) local_edges) / ((float) (G.getNodeDegree(node) * (G.getNodeDegree(node) - 1)));
        else
            getFeature<LCC>(node) = 0;
        avg_lcc += getFeature<LCC>(node);

        // local chromatic density
        forall_out_edges(G, edge, node) {
            used_colors[node_coloring[G.getEdgeTarget(edge)]] = true;
        } endfor
        getFeature<CHROMATIC>(node) = (float) std::accumulate(used_colors.begin(), used_colors.end(), 0) / (float) greedy_chromatic_number;

        // total weight
        getFeature<T_WEIGHT>(node) = (float) total_weight;

        // node weight
        getFeature<NODE_W>(node) = (float) G.getNodeWeight(node);

        // node weighted degree
        forall_out_edges(G, edge, node) {
            getFeature<W_DEG>(node) += (float) G.getNodeWeight(G.getEdgeTarget(edge));
        } endfor
        avg_wdeg = getFeature<W_DEG>(node);

        // local search
        getFeature<LOCAL_SEARCH>(node) += (float) ls_signal[node] / mis_config.ls_rounds;

    } endfor

    avg_lcc /= (float) G.number_of_nodes();
    avg_wdeg /= (float) G.number_of_nodes();

    // statistical features
    float avg_deg = (2 * (float) G.number_of_edges()) / ((float) G.number_of_nodes());

    forall_nodes(G,node) {
        getFeature<CHI2_DEG>(node) = (float) chi2(G.getNodeDegree(node), avg_deg);

        float avg_chi2_deg = 0;
        forall_out_edges(G, edge, node) {
            avg_chi2_deg += getFeature<AVG_CHI2_DEG>(node);
        } endfor

        if(G.getNodeDegree(node) > 0)    // catch divide by zero
            getFeature<AVG_CHI2_DEG>(node) = avg_chi2_deg / (float) G.getNodeDegree(node);
        else
            getFeature<AVG_CHI2_DEG>(node) = 0;

        getFeature<CHI2_LCC>(node) = (float) chi2(getFeature<LCC>(node), avg_lcc);

        getFeature<CHI2_W_DEG>(node) = (float) chi2(getFeature<W_DEG>(node), avg_wdeg);
    } endfor

    current_size += G.number_of_nodes();
}

void ml_features::initDMatrix() {
    XGDMatrixCreateFromMat(feature_matrix.c_arr(), feature_matrix.getRows(), feature_matrix.getCols(), 0, &dmat);
    XGDMatrixSetFloatInfo(dmat, "label", &label_data[0], label_data.size());
}

DMatrixHandle ml_features::getDMatrix() {
    return dmat;
}




