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

void ml_assign_weights(graph_access& G, const MISConfig& mis_config) {
    constexpr NodeWeight MAX_WEIGHT = 200;

    if (mis_config.weight_source == MISConfig::Weight_Source::HYBRID) {
        forall_nodes(G, node) {
            G.setNodeWeight(node, (node + 1) % MAX_WEIGHT + 1);
        } endfor
    } else if (mis_config.weight_source == MISConfig::Weight_Source::UNIFORM) {
        std::default_random_engine generator(mis_config.seed);
        std::uniform_int_distribution<NodeWeight> distribution(1,MAX_WEIGHT);

        forall_nodes(G, node) {
            G.setNodeWeight(node, distribution(generator));
        } endfor
    } else if (mis_config.weight_source == MISConfig::Weight_Source::GEOMETRIC) {
        std::default_random_engine generator(mis_config.seed);
        std::binomial_distribution<int> distribution(MAX_WEIGHT / 2);

        forall_nodes(G, node) {
            G.setNodeWeight(node, distribution(generator));
        } endfor
    }
}

// constructors
// for training (many graphs with labels)
ml_features::ml_features(const MISConfig& config)
    : has_labels {true}
{
    std::cout << "FEATURE_NUM " << FEATURE_NUM << std::endl;
    configuration_mis cfg;
    cfg.standard(mis_config);
    mis_config.console_log = config.console_log;
    mis_config.ml_pruning = config.ml_pruning;
    mis_config.time_limit = config.ls_time;
    mis_config.ls_rounds = config.ls_rounds;
}

// for reducing (single graph without labels)
ml_features::ml_features(const MISConfig& config, graph_access &G)
    : has_labels {false}
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
    std::cout << "LOG: ml-calcFeatures: " << graph_paths.size() << " of " << all_graph_paths.size() << " were non empty\n";

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
        std::cout << "LOG: ml-calcFeatures: graph " << graph_paths[i] << " (" << (float) i / graph_paths.size() * 100 << "%)\n";
        graph_access G;
        graph_io::readGraphWeighted(G, graph_paths[i]);
        ml_assign_weights(G, mis_config);
        std::vector<float> labels(G.number_of_nodes(), 0);
        graph_io::readVector<float>(labels.begin(), offsets[i+1]-offsets[i], label_paths[i]);

        fillGraph(G, labels, offsets[i]);
    }
}

// init
void ml_features::reserveNodes(NodeID n) {
    feature_matrix.resize(feature_matrix.size() + n);
    label_data.resize(label_data.size() + n);
}

void ml_features::fillGraph(graph_access& G) {
    if (has_labels) {
        std::cerr << "Error: feature matrix was constructed for labels, so provide the labels for the graph.\n";
        exit(1);
    }
    calculate_features(G);
}

void ml_features::fillGraph(graph_access& G, std::vector<float>& labels, NodeID offset) {
    if (!has_labels) {
        std::cerr << "Error: feature matrix was constructed not for labels, so the labels will be discarded.\n";
    }
    calculate_features(G);
    std::copy(labels.begin(), labels.end(), label_data.begin() + offset);
}



void ml_features::calculate_features(graph_access& G) {
    // precompute values

    NodeWeight total_weight = 0;
    NodeWeight min_weight = std::numeric_limits<NodeWeight>::max();

    NodeWeight max_weight = std::numeric_limits<NodeWeight>::min();

    // greedy node coloring
    std::vector<int> node_coloring(G.number_of_nodes());
    std::vector<bool> available(G.number_of_nodes(), true);

    forall_nodes(G, node) {
        NodeWeight weight = G.getNodeWeight(node);

#ifdef F_T_WEIGHT
        total_weight += weight;
#endif
#ifdef F_MIN_W
        min_weight = std::min(weight, min_weight);
#endif
#ifdef F_MAX_W
        max_weight = std::max(weight, max_weight);
#endif
#ifdef F_CHROMATIC
        std::fill(available.begin(), available.end(), true);
        forall_out_edges(G, edge, node) {
            available[node_coloring[G.getEdgeTarget(edge)]] = false;
        } endfor
        node_coloring[node] = (int) (std::find_if(available.begin(), available.end(), [](bool x){ return x; }) - available.begin());
#endif

    } endfor

#ifdef F_CHROMATIC
    int greedy_chromatic_number = *std::max_element(node_coloring.begin(), node_coloring.end()) + 1;
    std::vector<bool> used_colors(greedy_chromatic_number);
#endif

#ifdef F_LS
    // local search
    std::vector<int> ls_signal(G.number_of_nodes(), 0);
    std::random_device rd;

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
        // int count = 0;
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
#endif

    // loop variables
    EdgeID local_edges = 0;
    std::unordered_set<NodeID> neighbors {};      // don't know how to do faster? maybe using bitset from boost?
    neighbors.reserve(G.getMaxDegree()+1);
    float avg_lcc = 0;
    float avg_wdeg = 0;

    forall_nodes(G, node){
        // num of nodes/ edges, deg
#ifdef F_NODES
        get_feature<NODES>(node) = (float) G.number_of_nodes();
#endif
#ifdef F_EDGES
        get_feature<EDGES>(node) = (float) G.number_of_edges();
#endif
#ifdef F_DEG
        get_feature<DEG>(node) = (float) G.getNodeDegree(node);
#endif
#ifdef F_LCC
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
            get_feature<LCC>(node) = ((float) local_edges) / ((float) (G.getNodeDegree(node) * (G.getNodeDegree(node) - 1)));
        else
            get_feature<LCC>(node) = 0;
        avg_lcc += get_feature<LCC>(node);
#endif

        // local chromatic density and maximum weight in neighborhood
        NodeWeight max_neighborhood_weight = std::numeric_limits<NodeWeight>::min();
        NodeWeight sum_neighborhood_weight = 0;

        forall_out_edges(G, edge, node) {
            NodeID neighbor = G.getEdgeTarget(edge);

#ifdef F_CHROMATIC
            used_colors[node_coloring[neighbor]] = true;
#endif

#ifdef F_MAX_NEIGHBORHOOD_W
            max_neighborhood_weight = std::max(max_neighborhood_weight, G.getNodeWeight(neighbor));
#endif

#ifdef F_W_DEG
            sum_neighborhood_weight += G.getNodeWeight(neighbor);
#endif

        } endfor

#ifdef F_CHROMATIC
        get_feature<CHROMATIC>(node) = (float) std::accumulate(used_colors.begin(), used_colors.end(), 0) / (float) greedy_chromatic_number;
#endif
#ifdef F_MAX_NEIGHBORHOOD_W
        get_feature<MAX_NEIGHBORHOOD_W>(node) = max_neighborhood_weight;
#endif
#ifdef F_HT
        get_feature<HT>(node) = sum_neighborhood_weight - G.getNodeWeight(node);
#endif
#ifdef F_T_WEIGHT
        // total weight
        get_feature<T_WEIGHT>(node) = (float) total_weight;
#endif

#ifdef F_NODE_W
        // node weight
        get_feature<NODE_W>(node) = (float) G.getNodeWeight(node);
#endif

#ifdef F_W_DEG
        // node weighted degree
        forall_out_edges(G, edge, node) {
            get_feature<W_DEG>(node) += (float) G.getNodeWeight(G.getEdgeTarget(edge));
        } endfor
#endif
#ifdef F_CHI2_W_DEG
        avg_wdeg = get_feature<W_DEG>(node);
#endif
#ifdef F_LS
        // local search
        get_feature<LOCAL_SEARCH>(node) += (float) ls_signal[node] / mis_config.ls_rounds;
#endif

        // minimum and maximum overall weight
#ifdef F_MAX_W
        get_feature<MAX_W>(node) = max_weight;
#endif
#ifdef F_MIN_W
        get_feature<MIN_W>(node) = min_weight;
#endif
    } endfor

#ifdef F_CHI2_LCC
    avg_lcc /= (float) G.number_of_nodes();
#endif
#ifdef F_CHI2_W_DEG
    avg_wdeg /= (float) G.number_of_nodes();
#endif

    // statistical features
#ifdef F_AVG_CHI2_DEG
    float avg_deg = (2 * (float) G.number_of_edges()) / ((float) G.number_of_nodes());
#endif

    forall_nodes(G,node) {
#ifdef F_CHI2_DEG
        get_feature<CHI2_DEG>(node) = (float) chi2(G.getNodeDegree(node), avg_deg);
#endif

#ifdef F_AVG_CHI2_DEG
        float avg_chi2_deg = 0;
        forall_out_edges(G, edge, node) {
            avg_chi2_deg += get_feature<AVG_CHI2_DEG>(node);
        } endfor

        if(G.getNodeDegree(node) > 0)    // catch divide by zero
            get_feature<AVG_CHI2_DEG>(node) = avg_chi2_deg / (float) G.getNodeDegree(node);
        else
            get_feature<AVG_CHI2_DEG>(node) = 0;
#endif
#ifdef F_CHI2_LCC
        get_feature<CHI2_LCC>(node) = (float) chi2(get_feature<LCC>(node), avg_lcc);
#endif
#ifdef F_CHI2_W_DEG
        get_feature<CHI2_W_DEG>(node) = (float) chi2(get_feature<W_DEG>(node), avg_wdeg);
#endif
    } endfor

    current_size += G.number_of_nodes();
}

void ml_features::initDMatrix() {
    if (feature_matrix.empty()) {
        std::cerr << "feature matrix empty, cannot create DMatrix from it!" << std::endl;
        return;
    }
    XGDMatrixCreateFromMat(c_arr(feature_matrix), getRows(), getCols(), 0, &dmat);
    XGDMatrixSetFloatInfo(dmat, "label", &label_data[0], label_data.size());
}

DMatrixHandle ml_features::getDMatrix() {
    return dmat;
}

/*
 * For nodes from different graphs with equal features, average over the labels
 * to avoid conflicting information for the booster.
 */
void ml_features::regularize() {
    // iterators to rows
    std::vector<matrix::iterator> rows(getRows());
    std::iota(rows.begin(), rows.end(), feature_matrix.begin());

    // sort the rows by columns consecutively,
    // consecutive rows will then be approx equal
    std::sort(rows.begin(), rows.end(), [](auto row1, auto row2) {
        size_t col = 0;
        while (float_approx_eq((*row1)[col], (*row2)[col]) && col < FEATURE_NUM - 1)
            ++col;
        return row1[col] < row2[col];
    });

    // find all rows which are the same
    matrix regularized_feature_matrix;
    std::vector<float> regularized_label_data;
    {
        auto row = rows.begin();
        float label_sum = label_data.front();
        for (auto next = row + 1; next != rows.end(); next++) {
            size_t col = 0;
            while (float_approx_eq((**row)[col], (**next)[col]) && col < FEATURE_NUM)
                ++col;

            auto next_label = label_data[*next - feature_matrix.begin()];
            // if row and equal are not equal until the last column (last feature)
            if (col < FEATURE_NUM) {
                // copy the row
                regularized_feature_matrix.push_back(**row);
                // copy averaged label
                regularized_label_data.push_back(label_sum / (float) (next-row));
                // advance row to next
                row = next;
                // reset label_sum to only the label of the current row
                label_sum = next_label;
            }
            // if row and next are approx equal
            else {
                label_sum += next_label;
            }
        }
    }

    std::cout << "regularization: removed " << (float) (feature_matrix.size() - regularized_feature_matrix.size()) / feature_matrix.size() << "%" << std::endl;

    // update feature_matrix and label_data to the new regularized data
    feature_matrix = std::move(regularized_feature_matrix);
    current_size   = feature_matrix.size();
    label_data     = std::move(regularized_label_data);
}

bool ml_features::float_approx_eq(float a, float b) {
    return std::abs(a-b) < eps;
}

float ml_features::scale_pos_weight_param() const {
    size_t number_of_positive = 0;
    size_t number_of_negative = 0;
    for (const auto label : label_data) {
        if (label == 0)
            ++number_of_negative;
        if (label == 1)
            ++number_of_positive;
    }

    return (float) number_of_negative / (float) number_of_positive;
}
