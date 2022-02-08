//
// Created by joseph on 2/5/22.
//

#include <ml_features.h>
#include <mis_log.h>
#include <fstream>
#include <argtable3.h>
#include "mis_config.h"
#include "parse_parameters_train.h"

std::vector<std::string> read_lines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream ifstream(path);
    if (!ifstream) {
        std::cerr << "Could not open file " << path << "\n";
        exit(1);
    }
    std::string line;
    while (std::getline(ifstream, line)) {
        if (line[0] == '#')
            continue;
        lines.push_back(line);
    }
    return lines;
}

int main(int argn, char** argv) {
    mis_log::instance()->restart_total_timer();
    //mis_log::instance()->print_title();

    MISConfig mis_config;
    std::string graph_filepath;

    // Parse the command line parameters;
    int ret_code = parse_parameters_train(argn, argv, mis_config, graph_filepath);
    if (ret_code) {
        return ret_code;
    }

    std::string train_graphs_path("train_graphs_list.txt");
    std::string train_labels_path("train_labels_list.txt");
    std::string test_graph_path("test_graphs_list.txt");
    std::string test_label_path("test_labels_list.txt");

    // get combined feature matrices for training and testing graphs
    ml_features train_features(mis_config), test_features(mis_config);
    std::cout << "LOG: ml-train: getting feature matrices for training\n";

    train_features.fromPaths(read_lines(train_graphs_path),
                             read_lines(train_labels_path));
    std::cout << "LOG: ml-train: getting feature matrices for testing\n";
    test_features.fromPaths(read_lines(test_graph_path),
                            read_lines(test_label_path));

    // initialize DMatrices in with feature and label data
    train_features.initDMatrix();
    test_features.initDMatrix();

    // create booster
    BoosterHandle booster;
    DMatrixHandle eval_dmats[2] = {train_features.getDMatrix(), test_features.getDMatrix()};
    safe_xgboost(XGBoosterCreate(eval_dmats, 2, &booster));

    // parameters
    // no gpu  // TODO: change training to gpu?
    safe_xgboost(XGBoosterSetParam(booster, "gpu_id", "-1"));

    safe_xgboost(XGBoosterSetParam(booster, "objective", "binary:logistic"));
    safe_xgboost(XGBoosterSetParam(booster, "eval_metric", "logloss"));

    safe_xgboost(XGBoosterSetParam(booster, "gamma", "0.1"));
    safe_xgboost(XGBoosterSetParam(booster, "max_depth", "3"));
    safe_xgboost(XGBoosterSetParam(booster, "eta", "1"));
    safe_xgboost(XGBoosterSetParam(booster, "verbosity", "1"));

    std::cout << "LOG: ml-train: starting training\n";
    int n_trees = 10;
    const char *eval_names[2] = {"train", "test"};
    const char *eval_result = nullptr;

    for (int i = 0; i < n_trees; ++i) {
        std::cout << "LOG: ml-train: round " << i << "of training\n";
        safe_xgboost(XGBoosterUpdateOneIter(booster, i, train_features.getDMatrix()));
        safe_xgboost(XGBoosterEvalOneIter(booster, i, eval_dmats, eval_names, 2, &eval_result));
        printf("%s\n", eval_result);
    }

    bst_ulong num_feature = 0;
    safe_xgboost(XGBoosterGetNumFeature(booster, &num_feature));
    assert(num_feature == ml_features::FEATURE_NUM);

    // create timestamp name
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char time_stamp_name[200];
    strftime(time_stamp_name, sizeof(time_stamp_name), MODEL_DIR "/%y-%m-%d_%H-%M-%S.model", t);

    // saving model
    std::cout << "LOG: ml-train: saving model into " MODEL_DIR "/latest.model and " << time_stamp_name << "\n";
    safe_xgboost(XGBoosterSaveModel(booster, MODEL_DIR "/latest.model"));
    safe_xgboost(XGBoosterSaveModel(booster, mis_config.model.c_str() ));
    safe_xgboost(XGBoosterSaveModel(booster, time_stamp_name));

    XGDMatrixFree(eval_dmats[0]);
    XGDMatrixFree(eval_dmats[1]);
    XGBoosterFree(booster);
}