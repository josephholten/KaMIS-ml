/**
 * parse_parameters.h
 * Purpose: Parse command line parameters.
 *
 *****************************************************************************/

#ifndef _PARSE_PARAMETERS_ML_H_
#define _PARSE_PARAMETERS_ML_H_

#include <omp.h>
#include "argtable3.h"

#include "configuration_mis.h"
#include <filesystem>

/**
 * Parse the given parameters and apply them to the config.
 *
 * @param argn Number of parameters.
 * @param argv Values of the parameters.
 * @param mis_config Config to store the values in.
 * @param graph_filename String to store the filename of the graph.
 *
 * @return -1 if there was an error. 0 otherwise.
 */
int parse_parameters_ml(int argn, char **argv,
                     MISConfig & mis_config,
                     std::string & graph_filename) {
    const char *progname = argv[0];

    // Setup the argtable structs
    struct arg_lit *help                = arg_lit0(NULL, "help", "Print help.");
    struct arg_int *user_seed           = arg_int0(NULL, "seed", NULL, "Seed to use for the PRNG.");
    struct arg_str *filename            = arg_strn(NULL, NULL, "FILE", 1, 1, "Path to graph file.");
    struct arg_str *output              = arg_str0(NULL, "output", NULL, "Path to store resulting independent set.");
    struct arg_dbl *time_limit          = arg_dbl0(NULL, "time_limit", NULL, "Time limit in s. Default 1000s.");
    struct arg_lit *console_log         = arg_lit0(NULL, "console_log", "Stream the log into the console");
    struct arg_lit *disable_checks      = arg_lit0(NULL, "disable_checks", "Disable sortedness check during I/O.");
	struct arg_lit *random_freenodes    = arg_lit0(NULL, "random_freenodes", "Randomly picks free nodes to maximize to IS instead of sorting them by weight.");
	struct arg_lit *disable_reduction   = arg_lit0(NULL, "disable_reduction", "Don't perforn any reductions.");
    struct arg_str *weight_source       = arg_str0(NULL, "weight_source", NULL, "Choose how the weights are assigned. Can be either: file (default), hybrid, uniform, geometric.");
    struct arg_str *reduction_style     = arg_str0(NULL, "reduction_style", NULL, "Choose the type of reductions appropriate for the input graph. Can be either: normal/sparse (default), dense/osm.");
    struct arg_int *ls_rounds           = arg_int0(NULL, "ls_rounds", NULL, "Number of local search signals to compute for ML reductions");
    struct arg_dbl *ls_time             = arg_dbl0(NULL, "ls_time", NULL, "Time limit for local search signals in ML reductions");
    struct arg_lit *ls_updates          = arg_lit0(NULL, "ls_updates", "Print updates in local search signal in ML reductions");
    struct arg_dbl *ml_pruning          = arg_dbl0(NULL, "ml_pruning", NULL, "How aggresively to prune vertices based on the ML prediction");
    struct arg_str *model               = arg_str0(NULL, "model", NULL, "Where to load/ save the model");
    struct arg_str *log_file            = arg_str0(NULL, "log_file", NULL, "Path to the log file.");

    struct arg_end *end                 = arg_end(100);

    // Setup the argtable
    void *argtable[] = {
            help,
            filename,
            output,
            user_seed,
            time_limit,
            console_log,
            disable_checks,
			//random_freenodes,
			//disable_reduction,
            weight_source,
            reduction_style,
            ls_rounds,
            ls_time,
            ls_updates,
            ml_pruning,
            model,
            log_file,
            end
    };

    // Choose standard configuration
    configuration_mis cfg;
    cfg.standard(mis_config);

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

    if (filename->count > 0) {
        graph_filename = filename->sval[0];
    }

    if (user_seed->count > 0) {
        mis_config.seed = user_seed->ival[0];
    }

    if (time_limit->count > 0) {
        mis_config.time_limit = time_limit->dval[0];
    }

    if (console_log->count > 0) {
        mis_config.console_log = true;
        mis_config.print_log = false;
    } else {
        mis_config.print_log = true;
    }

    if (disable_checks->count > 0) {
        mis_config.check_sorted = false;
    }

	if (random_freenodes->count > 0) {
		mis_config.sort_freenodes = false;
	}

	if (disable_reduction->count > 0) {
		mis_config.perform_reductions = false;
	}

	if (weight_source->count > 0) {
		mis_config.setWeightSource(weight_source->sval[0]);
	}

	if (reduction_style->count > 0) {
		mis_config.setReductionStyle(reduction_style->sval[0]);
	}

    if (output->count > 0) {
        mis_config.output_filename = output->sval[0];
        mis_config.write_graph = true;
    } else {
        mis_config.write_graph = false;
    }

    if (ls_rounds->count > 0) {
        mis_config.ls_rounds = ls_rounds->ival[0];
    }

    if (ls_time->count > 0) {
        mis_config.ls_time = ls_time->dval[0];
    }

    if (ls_updates->count > 0) {
        mis_config.ls_updates = true;
    }

    if (ml_pruning->count > 0) {
        mis_config.ml_pruning = (float) ml_pruning->dval[0];
    }

    if (model->count > 0) {
        // check if absolute path
        std::filesystem::path model_path(model->sval[0]);
        if (model_path.is_absolute())
            mis_config.model = model_path.string();

        // otherwise assume relative to MODEL_DIR
        else {
            model_path = MODEL_DIR;
            model_path.append(model->sval[0]);
            mis_config.model = model_path.string();
        }
        // check if valid path
        if (std::ifstream model_file(model_path); !model_file) {
            std::cerr << "iterative-ml: ERROR: Please provide a valid ml model path." << std::endl;
            exit(1);
        }
    }

    if (log_file->count > 0) {
        mis_config.log_file = log_file->sval[0];
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return 0;
}

#endif
