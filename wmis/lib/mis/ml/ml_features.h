//
// Created by joseph on 10/27/21.
//

#ifndef MWIS_ML_ML_FEATURES_H
#define MWIS_ML_ML_FEATURES_H

#include <vector>
#include <mis_config.h>
#include <array>

#include "graph_access.h"
#include "util.h"
#include "feature_set_macros.h"
#include "weighted_dynamic_graph.h"

class weighted_dynamic_graph;

// The features used in the prediction of the random forest model need to be in a matrix format.
// The matrix is in a row-major format. The row of index i is the feature vector of the node with ID i.
// The class calculates the features and fills the matrix in the correct order.
class ml_features {
private:
    enum feature : size_t {
#ifdef F_NODES
        NODES,
#endif
#ifdef F_EDGES
        EDGES,
#endif
#ifdef F_DEG
        DEG,
#endif
#ifdef F_CHI2_DEG
        CHI2_DEG,
#endif
#ifdef F_AVG_CHI2_DEG
        AVG_CHI2_DEG,
#endif
#ifdef F_LCC
        LCC,
#endif
#ifdef F_CHI2_LCC
        CHI2_LCC,
#endif
#ifdef F_CHROMATIC
        CHROMATIC,
#endif
#ifdef F_T_WEIGHT
        T_WEIGHT,
#endif
#ifdef F_NODE_W
        NODE_W,
#endif
#ifdef F_W_DEG
        W_DEG,
#endif
#ifdef F_CHI2_W_DEG
        CHI2_W_DEG,
#endif
#ifdef F_MAX_W
        MAX_W,
#endif
#ifdef F_MIN_W
        MIN_W,
#endif
#ifdef F_MAX_NEIGHBORHOOD_W
        MAX_NEIGHBORHOOD_W,
#endif
#ifdef F_HT
        HT,
#endif
        FEATURE_NUM_ENUM};

public:
    static constexpr int FEATURE_NUM = FEATURE_NUM_ENUM;
    using matrix = std::vector<std::array<float, FEATURE_NUM>>;

    // Create the feature matrix for a weighted_dynamic graph and a subset of nodes
    explicit ml_features(const MISConfig& config, const weighted_dynamic_graph &G, const std::vector<NodeID> &nodes);
    // Create the feature matrix for a single graph
    explicit ml_features(const MISConfig& config, graph_access& G);
    // Initialize an empty feature matrix to be filled by multiple graphs (all training data needs to be a single matrix)
    explicit ml_features(const MISConfig& config);
    ~ml_features();

    // Getters
    [[nodiscard]] size_t getRows() const { return feature_matrix.size(); };
    [[nodiscard]] static constexpr size_t getCols() { return FEATURE_NUM; };

    // Fill the matrix from vectors of matching paths to graphs and their corresponding labels
    void fromPaths(const std::vector<std::string> &graphs, const std::vector<std::string> &labels);

    static bool float_approx_eq(float a, float b);

    // Reserve n rows
    void reserveNodes(NodeID n);
    // Fill the matrix for graph G
    void fillGraph(graph_access& G);
    // Fill the matrix for a subset of nodes of graph G
    void fillGraph(const weighted_dynamic_graph& G, const std::vector<NodeID>& nodes);
    // Fill the matrix for graph G and corresponding labels, starting at offset.
    void fillGraph(graph_access& G, std::vector<float>& labels, NodeID offset);

    // Used for interfacing with the XGBoost C-API
    void initDMatrix();
    DMatrixHandle getDMatrix();

    // Return reference to the row of a node
    const std::array<float, FEATURE_NUM>& getRow(NodeID node) { return feature_matrix[node]; }

    std::vector<std::array<float, FEATURE_NUM>>::iterator begin() { return feature_matrix.begin(); }
    std::vector<std::array<float, FEATURE_NUM>>::iterator end() { return feature_matrix.end(); }

    // Used for equally sampling positive and negative labels in training
    [[nodiscard]] float scale_pos_weight_param() const;

    void regularize();
    void normalize();
    void min_max_normalize();
    void to_file(std::string filepath);

private:
    static_assert(ml_features::FEATURE_NUM == FEATURE_NUM_ENUM, "assure ml_features::FEATURE_NUM is correct");
    static constexpr float eps = 1e-4;

    MISConfig mis_config;
    bool has_labels;

    size_t current_size {0};
    matrix feature_matrix;
    inline float* c_arr(const matrix& m) { return feature_matrix[0].data(); };
    std::vector<float> label_data;

    void calculate_features(graph_access &G);
    void calculate_features(const weighted_dynamic_graph &G, const std::vector<NodeID> &nodes);

    template<feature f>
    float& get_feature(NodeID node) {
        return feature_matrix[node + current_size][f];
    }

    DMatrixHandle dmat;
};

#endif //MWIS_ML_ML_FEATURES_H