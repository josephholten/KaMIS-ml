/**
 * parse_parameters.h
 * Purpose: Parse command line parameters.
 *
 *****************************************************************************/

#ifndef _PARSE_PARAMETERS_H_
#define _PARSE_PARAMETERS_H_

#include <omp.h>
#include <argtable3.h>
#include <random>
#include <map>
#include "../../extern/KaHIP/lib/tools/random_functions.h"


static bool str_cmp(const std::string & str1, const std::string & str2) {
    return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2){ return std::toupper(c1) == std::toupper(c2); });
}

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
    NodeID max_size = 2;
    NodeWeight max_weight = 1;
    double bias = 2;
    int seed = 1;

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
        file << "# max_size " << max_size << std::endl;
        file << "# max_weight " << max_weight << std::endl;
        file << "# bias " << bias << std::endl;
        file << "# weight_source " << weight_source_to_str.at(source) << std::endl;
    }
};

/**
 * Parse the given parameters and apply them to the config.
 *
 * @param argn Number of parameters.
 * @param argv Values of the parameters.
 * @param config Config to store the values in.
 * @param graph_filename String to store the filename of the graph.
 *
 * @return -1 if there was an error. 0 otherwise.
 */
int parse_parameters(int argn, char **argv,
                     synth_config& config,
                     std::string& path) {
    const char *progname = argv[0];

    // Setup the argtable structs
    struct arg_lit *help                = arg_lit0(NULL, "help", "Print help.");
    struct arg_int *user_seed           = arg_int0(NULL, "seed", NULL, "Seed to use for the PRNG.");
    struct arg_str *incomplete_path     = arg_strn(NULL, NULL, "FILE", 1, 1, "Incomplete path to output graph files.");
    struct arg_str *output              = arg_str0(NULL, "output", NULL, "Path to store resulting independent set.");
    struct arg_dbl *time_limit          = arg_dbl0(NULL, "time_limit", NULL, "Time limit in s. Default 1000s.");
    struct arg_lit *console_log         = arg_lit0(NULL, "console_log", "Stream the log into the console");
    struct arg_lit *disable_checks      = arg_lit0(NULL, "disable_checks", "Disable sortedness check during I/O.");
	struct arg_lit *random_freenodes    = arg_lit0(NULL, "random_freenodes", "Randomly picks free nodes to maximize to IS instead of sorting them by weight.");
	struct arg_lit *disable_reduction   = arg_lit0(NULL, "disable_reduction", "Don't perforn any reductions.");
    struct arg_str *weight_source       = arg_str0(NULL, "weight_source", NULL, "Choose how the weights are assigned. Can be either: file (default), hybrid, uniform, geometric.");
    struct arg_str *reduction_style     = arg_str0(NULL, "reduction_style", NULL, "Choose the type of reductions appropriate for the input graph. Can be either: normal/sparse (default), dense/osm.");
    struct arg_int *max_size            = arg_int0(NULL, "max_size", NULL, "The maximum size of each synthetic graph");
    struct arg_int *max_weight          = arg_int0(NULL, "max_weight", NULL, "The maximum weight of a node.");
    struct arg_dbl *bias                = arg_dbl0(NULL, "bias", NULL, "Ratio between weight of center and external vertices in stars.");
    struct arg_str *type                = arg_strn(NULL, NULL, "type", 1, 4, "Which types of synthetic graphs to generate.");
    struct arg_int *instances           = arg_int0(NULL, "instances", NULL, "How many graphs of each type to generate");
    struct arg_end *end                 = arg_end(100);

    // Setup the argtable
    void *argtable[] = {
            help,
            incomplete_path,
            // output,
            user_seed,
            // time_limit,
            // console_log,
            // disable_checks,
			//random_freenodes,
			//disable_reduction,
            weight_source,
            max_size,
            type,
            max_weight,
            bias,
            instances,
            // reduction_style,
            end
    };

    // Parse the arguments
    int nerrors = arg_parse(argn, argv, argtable);

    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-40s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }

    if (nerrors > 0) {
        arg_print_errors(stderr, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }

    if (incomplete_path->count > 0) {
        auto idx = std::string(incomplete_path->sval[0]).rfind('/');
        path = std::string(incomplete_path->sval[0]).substr(0,idx);
    }

    if (user_seed->count > 0) {
        config.seed = user_seed->ival[0];
    } else {
// #ifdef NDEBUG
        std::random_device rng;
        config.seed = rng();
// #endif
    }

    if (bias->count > 0) {
        config.bias = bias->dval[0];
    }

    if (max_size->count > 0) {
        assert(max_size->ival[0] >= 2 && "max_size >= 2");
        config.max_size = max_size->ival[0];
    }

    if (max_weight->count > 0) {
        config.max_weight = max_size->ival[0];
    }

    for (int i = 0; i < type->count; ++i) {
        if (str_cmp(type->sval[i], "path"))
            config.types.push_back(graph_family::path);
        else if (str_cmp(type->sval[i], "cycle"))
            config.types.push_back(graph_family::cycle);
        else if (str_cmp(type->sval[i], "star"))
            config.types.push_back(graph_family::star);
        else if (str_cmp(type->sval[i], "tree"))
            config.types.push_back(graph_family::tree);
        else
            std::cerr << "ignoring unknown type " << type->sval[i] << std::endl;
    }

    random_functions::setSeed(config.seed);

    if (weight_source->count > 0) {
		config.setWeightSource(weight_source->sval[0]);
	}

    if (instances->count > 0) {
        config.instances = instances->ival[0];
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return 0;
}

#endif
