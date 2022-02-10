//
// Created by joseph on 10/27/21.
//

#ifndef MWIS_ML_ML_FEATURES_H
#define MWIS_ML_ML_FEATURES_H

#include <vector>
#include <mis_config.h>

#include "graph_access.h"
#include "safe_c_api.h"

class ml_features {
public:
    static constexpr int FEATURE_NUM = 13;

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

    void regularize();

private:
    using matrix = std::vector<std::array<float, FEATURE_NUM>>;
    enum feature : int { NODES=0, EDGES=1, DEG=2, CHI2_DEG=3, AVG_CHI2_DEG=4, LCC=5, CHI2_LCC=6, CHROMATIC=7, T_WEIGHT=8, NODE_W=9, W_DEG=10, CHI2_W_DEG=11, LOCAL_SEARCH=12, FEATURE_NUM_ENUM };
    static_assert(ml_features::FEATURE_NUM == FEATURE_NUM_ENUM, "assure ml_features::FEATURE_NUM is correct");
    static constexpr float eps = 1e-4;

    MISConfig mis_config;
    bool has_labels;

    size_t current_size {0};
    matrix feature_matrix;
    inline float* c_arr(const matrix& m) { return feature_matrix[0].data(); };
    std::vector<float> label_data;

    template<feature f>
    float& getFeature(NodeID node);

    void calcFeatures(graph_access& G);

    DMatrixHandle dmat;
};

#endif //MWIS_ML_ML_FEATURES_H