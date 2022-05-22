//
// Created by joseph on 10/22/21.
//

#ifndef MWIS_ML_SAFE_C_API_H
#define MWIS_ML_SAFE_C_API_H

#include "util.h"
#include <fstream>

bool validate_path(const std::string& path, bool weak=false) {
    //      file exists already               file does not yet exist
    if (std::ifstream ifile(path); !ifile.is_open()) {
        if (std::ofstream ofile(path); !ofile.is_open()) {
            std::perror("ERROR: invalid path");
            if (!weak)
                std::exit(1);
            else
                return false;
        } else
            std::remove(path.c_str());
    }
    return true;
}


#endif //MWIS_ML_SAFE_C_API_H
