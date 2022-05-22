//
// Created by joseph on 10/22/21.
//

#ifndef MWIS_ML_SAFE_C_API_H
#define MWIS_ML_SAFE_C_API_H

#include "xgboost/c_api.h"

#define safe_xgboost(call) {                                            \
int err = (call);                                                       \
if (err != 0) {                                                         \
    throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                        ": error in " + #call  + ":" + XGBGetLastError()); \
    }                                                                    \
}

bool validate_path(std::string path, bool weak=false) {
    //      file exists already               file does not yet exist
    if (!std::ifstream(path).is_open() && !std::ofstream(path).is_open()) {
        std::perror("ERROR: invalid path");
        std::exit(1);
    }
}

#endif //MWIS_ML_SAFE_C_API_H
