/******************************************************************************
 * graph_io.h
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef GRAPHIO_H_
#define GRAPHIO_H_

#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "definitions.h"
#include "data_structure/graph_access.h"
#include "algo_log.h"

class graph_io {
        public:
                graph_io();
                virtual ~graph_io () ;

                static
                int readGraphWeighted(graph_access & G, const std::string & filename);

                static
                NodeID readNumberOfNodes(const std::string & filename);

                static 
                int readGraphWeighted(graph_access & G, const std::string & filename, std::string & comments);

                static
                int writeGraphWeighted(graph_access & G, const std::string & filename);

                static
                int writeGraphNodeWeighted(graph_access & G, std::string filename);

                static
                int writeGraphNodeWeighted(graph_access & G, std::ofstream & file);      

                static
                int writeGraph(graph_access & G, const std::string & filename);

                static
                int readPartition(graph_access& G, const std::string & filename);

                static
                void writePartition(graph_access& G, const std::string & filename);

                static 
                void writeIndependentSet(graph_access & G, std::string filename);

                template<typename vectortype>
                static void writeVector(const std::vector<vectortype> & vec, const std::string & filename);

                template<typename vectortype>
                static void readVector(std::vector<vectortype> & vec, const std::string & filename);

                template<typename vectortype>
                static void readVector(typename std::vector<vectortype>::iterator vec, NodeID len, const std::string & filename);
};

template<typename vectortype>
void graph_io::writeVector(const std::vector<vectortype> & vec, const std::string & filename) {
        std::ofstream f(filename.c_str());
        for( unsigned i = 0; i < vec.size(); ++i) {
                f << vec[i] <<  std::endl;
        }
        algo_log::logger()["solution_file"] = filename;
}

template<typename vectortype>
void graph_io::readVector(std::vector<vectortype> & vec, const std::string & filename) {

        std::string line;

        // open file for reading
        std::ifstream in(filename.c_str());
        if (!in) {
                std::cerr << "Error opening vectorfile " << filename << std::endl;
                exit(1);
        }

        unsigned pos = 0;
        std::getline(in, line);
        while( !in.eof() ) {
                if (line[0] == '%') { //Comment
                        continue;
                }

                auto value = (vectortype) atof(line.c_str());
                vec[pos++] = value;
                std::getline(in, line);
        }

        in.close();
}

template<typename vectortype>
void graph_io::readVector(typename std::vector<vectortype>::iterator vec, NodeID len, const std::string & filename) {

    std::string line;

    // open file for reading
    std::ifstream in(filename.c_str());
    if (!in) {
        std::cerr << "Error opening vectorfile " << filename << std::endl;
        exit(1);
    }

    unsigned pos = 0;
    std::getline(in, line);
    while( !in.eof() && pos < len) {
        if (line[0] == '%') { //Comment
            continue;
        }
        if (pos >= len) {
            std::cerr << "File `" << filename << "` longer than specified len (" << len << ")!";
            break;
        }

        auto value = (vectortype) atof(line.c_str());
        vec[pos++] = value;
        std::getline(in, line);
    }

    in.close();
}

#endif /*GRAPHIO_H_*/
