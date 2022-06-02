//
// Created by joseph on 6/2/22.
//

#ifndef KAMIS_SYNTHETIC_CONFIG_H
#define KAMIS_SYNTHETIC_CONFIG_H

#include <string>
#include "definitions.h"
#include <map>
#include "util.h"

enum class graph_family { path, cycle, star, tree, cliques};
const std::map<graph_family, std::string> family_to_str {
        {graph_family::path, "path"},
        {graph_family::cycle, "cycle"},
        {graph_family::star, "star"},
        {graph_family::tree, "tree"},
};


struct synth_config {
    enum class weight_source {UNWEIGHTED, HYBRID, UNIFORM, GEOMETRIC};
    const std::map<weight_source, std::string> weight_source_to_str {
            {weight_source::UNWEIGHTED, "unweighted"},
            {weight_source::HYBRID, "hybrid"},
            {weight_source::UNIFORM, "uniform"},
            {weight_source::GEOMETRIC, "geometric"},
    };

    std::vector<graph_family> types;
    weight_source source = weight_source::UNWEIGHTED;

    int instances = 1;
    NodeWeight max_weight = 1;
    uint seed = 1;
    int componets = 0;

    void setWeightSource(const std::string & s) {
        if (str_cmp(s, "hybrid")) {
            source = weight_source::HYBRID;
        } else if (str_cmp(s, "uniform")) {
            source = weight_source::UNIFORM;
        } else if (str_cmp(s, "geometric")) {
            source = weight_source::GEOMETRIC;
        }
    }

    void serialize(std::ofstream& file) {
        file << "% components " << componets << std::endl;
        file << "% max_weight " << max_weight << std::endl;
        file << "% weight_source " << weight_source_to_str.at(source) << std::endl;
    }

    NodeID size;
};

#endif //KAMIS_SYNTHETIC_CONFIG_H
