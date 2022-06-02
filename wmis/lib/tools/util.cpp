//
// Created by joseph on 10/22/21.
//

#include "util.h"
#include <fstream>
#include <iostream>

bool validate_path(const std::string& path, bool weak=false) {
    //      file exists already               file does not yet exist
    if (std::ifstream ifile(path); !ifile.is_open()) {
        if (std::ofstream ofile(path); !ofile.is_open()) {
            std::cerr << "ERROR: invalid path: " << path << std::endl;
            if (!weak)
                std::exit(1);
            else
                return false;
        } else
            std::remove(path.c_str());
    }
    return true;
}

static bool str_cmp(const std::string & str1, const std::string & str2) {
    return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2){ return std::toupper(c1) == std::toupper(c2); });
}