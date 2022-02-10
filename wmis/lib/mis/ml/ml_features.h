//
// Created by joseph on 10/27/21.
//

#ifndef MWIS_ML_ML_FEATURES_H
#define MWIS_ML_ML_FEATURES_H

#include <vector>
#include <mis_config.h>

#include "graph_access.h"
#include "safe_c_api.h"

class matrix {
public:
    // TODO: add row reserving constructor
    explicit matrix(size_t _cols) : cols {_cols} {};
    matrix& operator=(matrix&& other)  noexcept {
        data = std::move(other.data);
        return *this;
    }

    inline void addRows(size_t _rows) {
        rows += _rows;
        data.resize(data.size() + rows*cols);
    }

    inline void addRows(float* start, size_t _rows) {
        rows += _rows;
        data.reserve(data.size() + rows*cols);
        std::copy(start, start+_rows*cols, std::back_inserter(data));
    }

    inline std::vector<float>::iterator operator[](size_t idx) { return data.begin() + idx*cols; };

    [[nodiscard]] inline size_t getRows() const { return rows; };
    [[nodiscard]] inline size_t getCols() const { return cols; };

    inline float* c_arr() { return data.data(); };

private:
    size_t cols {};
    size_t rows {};

    std::vector<float> data;
};

// TODO: possibly inherit from matrix?
class ml_features {
public:
    static constexpr int FEATURE_NUM = 13;

    explicit ml_features(const MISConfig& config, graph_access& G);   // for single graph
    explicit ml_features(const MISConfig& config);    // for multiple graphs
    ~ml_features();

    [[nodiscard]] static int getNumberOfFeatures() ;
    [[nodiscard]] size_t getRows() const { return feature_matrix.getRows(); };
    [[nodiscard]] size_t getCols() const { return feature_matrix.getCols(); };

    void fromPaths(const std::vector<std::string> &graphs, const std::vector<std::string> &labels);
    static bool float_approx_eq(float a, float b);

    void reserveNodes(NodeID n);   // for reserving memory
    void fillGraph(graph_access& G);
    void fillGraph(graph_access& G, std::vector<float>& labels, NodeID offset);

    void initDMatrix();
    DMatrixHandle getDMatrix();

    void regularize();

private:
    enum feature : int { NODES=0, EDGES=1, DEG=2, CHI2_DEG=3, AVG_CHI2_DEG=4, LCC=5, CHI2_LCC=6, CHROMATIC=7, T_WEIGHT=8, NODE_W=9, W_DEG=10, CHI2_W_DEG=11, LOCAL_SEARCH=12, FEATURE_NUM_ENUM };
    static_assert(ml_features::FEATURE_NUM == FEATURE_NUM_ENUM, "assure ml_features::FEATURE_NUM is correct");

    static constexpr float eps = 1e-4;

    MISConfig mis_config;
    size_t current_size {0};
    matrix feature_matrix;
    bool has_labels;
    std::vector<float> label_data;

    template<feature f>
    float& getFeature(NodeID node);

    void features(graph_access& G);

    DMatrixHandle dmat;
};

#endif //MWIS_ML_ML_FEATURES_H