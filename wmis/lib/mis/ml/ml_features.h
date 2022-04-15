//
// Created by joseph on 10/27/21.
//

#ifndef MWIS_ML_ML_FEATURES_H
#define MWIS_ML_ML_FEATURES_H

#include <vector>
#include <mis_config.h>

#include "graph_access.h"
#include "safe_c_api.h"
#include "feature_set_macros.h"

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

    explicit ml_features(const MISConfig& config, graph_access& G);   // for single graph
    explicit ml_features(const MISConfig& config);    // for multiple graphs
    ~ml_features();

    [[nodiscard]] size_t getRows() const { return feature_matrix.size(); };
    [[nodiscard]] static constexpr size_t getCols() { return FEATURE_NUM; };

    void fromPaths(const std::vector<std::string> &graphs, const std::vector<std::string> &labels);
    static bool float_approx_eq(float a, float b);

    void reserveNodes(NodeID n);   // for reserving memory
    void fillGraph(graph_access& G);
    void fillGraph(graph_access& G, std::vector<float>& labels, NodeID offset);

    void initDMatrix();
    DMatrixHandle getDMatrix();

    [[nodiscard]] float scale_pos_weight_param() const;

    void regularize();

private:
    using matrix = std::vector<std::array<float, FEATURE_NUM>>;
    static_assert(ml_features::FEATURE_NUM == FEATURE_NUM_ENUM, "assure ml_features::FEATURE_NUM is correct");
    static constexpr float eps = 1e-4;

    MISConfig mis_config;
    bool has_labels;

    size_t current_size {0};
    matrix feature_matrix;
    inline float* c_arr(const matrix& m) { return feature_matrix[0].data(); };
    std::vector<float> label_data;

    void calculate_features(graph_access& G);

    template<feature f>
    float& get_feature(NodeID node) {
        return feature_matrix[node + current_size][f];
    }

    DMatrixHandle dmat;
};

#endif //MWIS_ML_ML_FEATURES_H