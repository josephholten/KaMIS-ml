//
// Created by joseph on 2/8/22.
//

#include <graph_access.h>
#include <graph_io.h>
#include <xgboost/c_api.h>
#include <util.h>
#include <sstream>
#include <unordered_set>

bool strCompare(const std::string & str1, const std::string & str2) {
    return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2){ return std::toupper(c1) == std::toupper(c2); });
}

bool is_IS(graph_access& G) {
    forall_nodes(G, node) {
        if (G.getPartitionIndex(node) == 1) {
            forall_out_edges(G, edge, node) {
                NodeID neighbor = G.getEdgeTarget(edge);
                if (G.getPartitionIndex(neighbor) == 1) {
                    return false;
                }
            } endfor
        }
    } endfor

    return true;

}

int main(int argc, char** argv) {
    if (argc == 2) {
        graph_access G;
        graph_io::readGraphWeighted(G, argv[1]);

        std::vector<std::unordered_set<NodeID>> edge_sets(G.number_of_nodes());

        // Check for duplicate or self-loop edges
        bool duplicates = false;
        bool selfloops = false;
        forall_nodes(G, node) {
            forall_out_edges(G, edge, node) {
                if (G.getEdgeTarget(edge) == node)
                    selfloops = true;

                if (!edge_sets[node].insert(G.getEdgeTarget(edge)).second)
                    // if it already exists
                    duplicates = true;
            } endfor
        } endfor

        std::cout << "duplicates: " << (duplicates ? "true" : "false") << std::endl;
        std::cout << "self loops: " << (selfloops ? "true" : "false") << std::endl;

        // Check whether graph is undirected
        bool undirected = true;
        for(size_t node = 0; node < edge_sets.size() && undirected; ++node) {
            for(auto neighbor : edge_sets[node]) {
                if (edge_sets[neighbor].find(node) == edge_sets[neighbor].end()) {
                    undirected = false;
                }
            }
        }

        std::cout << "undirected: " << (undirected ? "true" : "false") << std::endl;

        return 0;
    }

    if (argc < 3) {
        std::cerr << "Must provide a method and at least one file." << std::endl;
        return 1;
    }

    // analyze XGBoost model
    if (strCompare(argv[1],"model")) {
        BoosterHandle booster;
        safe_xgboost(XGBoosterCreate(nullptr, 0, &booster));
        safe_xgboost(XGBoosterLoadModel(booster, argv[2]))
        bst_ulong features;
        safe_xgboost(XGBoosterGetNumFeature(booster, &features));
        std::cout << "model has " << features << " features" << std::endl;
        std::vector<std::string> types = {"weight", "gain", "cover", "total_gain", "total_cover"};
        bst_ulong out_n_features = 13;
        auto out_features = (char const**) malloc(60);
        bst_ulong out_dim = 1;
        auto out_shape = (bst_ulong const*) malloc(60);
        auto out_scores = (float const*) malloc(60);
        for (auto type : types) {
            std::stringstream config;
            config << "{\"importance_type\":" << "\"" <<  type << "\","
                   << R"("feature_names": ["NODES", "EDGES", "DEG", "CHI2_DEG", "AVG_CHI2_DEG", "LCC", "CHI2_LCC", "CHROMATIC", "T_WEIGHT", "NODE_W", "W_DEG", "CHI2_W_DEG", "LOCAL_SEARCH"]})";
            safe_xgboost(XGBoosterFeatureScore(booster, config.str().c_str(), &out_n_features, &out_features, &out_dim, &out_shape, &out_scores));
            std::stringstream file_name;
            file_name << MODEL_DIR << "/score_" << type << ".txt";
            std::vector<float> scores(out_scores, out_scores + out_n_features);
            std::vector<std::string> feature_names;
            for (size_t index = 0; index < out_n_features; ++index) {
                feature_names.emplace_back(*(out_features+index));
            }
            std::ofstream output_file(file_name.str());
            for (int i = 0; i < out_n_features; ++i) {
                output_file << feature_names[i] << " " << scores[i] << std::endl;
            }
        }
        // free(out_features);
        // free((void *) out_shape);
        // free((void *) out_scores);
        return 0;
    }

    // check whether set is independent
    if (strCompare(argv[1],"mis")) {
        if (argc != 5) {
            std::cerr << "For mis analysis you must first provide a graph, then a IS and it's type" << std::endl;
            return 1;
        }

        std::cout << "Reading graph: " << argv[2] << std::endl;
        graph_access G;
        graph_io::readGraphWeighted(G, argv[2]);


        std::cout << "Reading IS: " << argv[3] << std::endl;
        std::ifstream IS_file(argv[3]);

        if (strCompare(argv[4], "kamis")) {
            std::string IS_str(std::istreambuf_iterator<char>{IS_file}, {});
            for (std::size_t index = 0; index < IS_str.size(); index += 2) {
                G.setPartitionIndex(index / 2, IS_str[index] - '0');
            }
        }

        if (strCompare(argv[4], "amazon")) {
            NodeID node;
            while (IS_file >> node) {
                G.setPartitionIndex(node, 1);
            }
        }


        std::cout << "Checking IS: " << argv[3] << std::endl;
        bool IS = is_IS(G);

        std::cout << "Independent set check: " << (IS ? "passed" : "failed") << std::endl;
        return IS ? 0 : 1;
    }

    std::cerr << "Must specify a valid method" << std::endl;

    return 1;

}
