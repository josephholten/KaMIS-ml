cmake_minimum_required(VERSION 2.8.9)
project(components)

if (NOT CMAKE_BUILD_TYPE)
    message("No BUILD_TYPE, setting to Release")
    set(CMAKE_BUILD_TYPE Release) 
endif (NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message("Debug build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fPIC -O0")
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

if (CMAKE_BUILD_TYPE STREQUAL Release)
    message("Release build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -march=native -DNDEBUG -fPIC -O3")
endif(CMAKE_BUILD_TYPE STREQUAL Release)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -fopenmp")

CHECK_CXX_COMPILER_FLAG(-Wno-unused-value COMPILER_SUPPORTS_NOUNUSED)
if(COMPILER_SUPPORTS_NOUNUSED)
add_definitions(-Wno-unused-value)
endif()

CHECK_CXX_COMPILER_FLAG(-Wno-unused-value COMPILER_SUPPORTS_NOUNUSEDRES)
if(COMPILER_SUPPORTS_NOUNUSEDRES)
add_definitions(-Wno-unused-result)
endif()

CHECK_CXX_COMPILER_FLAG(-fno-stack-limit COMPILER_SUPPORTS_FNOSTACKLIMITS)
if(COMPILER_SUPPORTS_FNOSTACKLIMITS)
  add_definitions(-fno-stack-limit)
endif()

CHECK_CXX_COMPILER_FLAG(-funroll-loops COMPILER_SUPPORTS_FUNROLL_LOOPS)
if(COMPILER_SUPPORTS_FUNROLL_LOOPS)
  add_definitions(-funroll-loops)
endif()

if(FEATURE_SET)
    add_compile_definitions(FEATURE_SET=${FEATURE_SET})
    message("Using " ${FEATURE_SET} " feature set")
endif()

add_compile_definitions(MODEL_DIR="${CMAKE_SOURCE_DIR}/models")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/argtable3-3.0.3)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/io)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/partition)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/tools)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/KaHIP)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/KaHIP/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/KaHIP/lib/io)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/KaHIP/lib/partition)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/KaHIP/lib/tools)


set(LIBKAFFPA_SOURCE_FILES
  extern/KaHIP/lib/data_structure/graph_hierarchy.cpp
  extern/KaHIP/lib/algorithms/strongly_connected_components.cpp
  extern/KaHIP/lib/algorithms/topological_sort.cpp
  extern/KaHIP/lib/algorithms/push_relabel.cpp
  extern/KaHIP/lib/io/graph_io.cpp
  extern/KaHIP/lib/tools/quality_metrics.cpp
  extern/KaHIP/lib/tools/random_functions.cpp
  extern/KaHIP/lib/tools/graph_extractor.cpp
  extern/KaHIP/lib/tools/misc.cpp
  extern/KaHIP/lib/tools/partition_snapshooter.cpp
  extern/KaHIP/lib/partition/graph_partitioner.cpp
  extern/KaHIP/lib/partition/w_cycles/wcycle_partitioner.cpp
  extern/KaHIP/lib/partition/coarsening/coarsening.cpp
  extern/KaHIP/lib/partition/coarsening/contraction.cpp
  extern/KaHIP/lib/partition/coarsening/edge_rating/edge_ratings.cpp
  extern/KaHIP/lib/partition/coarsening/matching/matching.cpp
  extern/KaHIP/lib/partition/coarsening/matching/random_matching.cpp
  extern/KaHIP/lib/partition/coarsening/matching/gpa/path.cpp
  extern/KaHIP/lib/partition/coarsening/matching/gpa/gpa_matching.cpp
  extern/KaHIP/lib/partition/coarsening/matching/gpa/path_set.cpp
  extern/KaHIP/lib/partition/coarsening/clustering/node_ordering.cpp
  extern/KaHIP/lib/partition/coarsening/clustering/size_constraint_label_propagation.cpp
  extern/KaHIP/lib/partition/initial_partitioning/initial_partitioning.cpp
  extern/KaHIP/lib/partition/initial_partitioning/initial_partitioner.cpp
  extern/KaHIP/lib/partition/initial_partitioning/initial_partition_bipartition.cpp
  extern/KaHIP/lib/partition/initial_partitioning/initial_refinement/initial_refinement.cpp
  extern/KaHIP/lib/partition/initial_partitioning/bipartition.cpp
  extern/KaHIP/lib/partition/initial_partitioning/initial_node_separator.cpp
  extern/KaHIP/lib/partition/uncoarsening/uncoarsening.cpp
  extern/KaHIP/lib/partition/uncoarsening/separator/area_bfs.cpp
  extern/KaHIP/lib/partition/uncoarsening/separator/vertex_separator_algorithm.cpp
  extern/KaHIP/lib/partition/uncoarsening/separator/vertex_separator_flow_solver.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/cycle_improvements/greedy_neg_cycle.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/cycle_improvements/problem_factory.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/cycle_improvements/augmented_Qgraph.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/mixed_refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/label_propagation_refinement/label_propagation_refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/2way_fm_refinement/two_way_fm.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/two_way_flow_refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/boundary_bfs.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/flow_solving_kernel/cut_flow_problem_solver.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/most_balanced_minimum_cuts/most_balanced_minimum_cuts.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/complete_boundary.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/partial_boundary.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_scheduling/quotient_graph_scheduling.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_scheduling/simple_quotient_graph_scheduler.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_scheduling/active_block_quotient_graph_scheduler.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/kway_graph_refinement/kway_graph_refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/kway_graph_refinement/kway_graph_refinement_core.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/kway_graph_refinement/kway_graph_refinement_commons.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/cycle_improvements/augmented_Qgraph_fabric.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/cycle_improvements/advanced_models.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/kway_graph_refinement/multitry_kway_fm.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/node_separators/greedy_ns_local_search.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/node_separators/fm_ns_local_search.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/node_separators/localized_fm_ns_local_search.cpp
  extern/KaHIP/lib/algorithms/cycle_search.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/cycle_improvements/cycle_refinement.cpp
  extern/KaHIP/lib/partition/uncoarsening/refinement/tabu_search/tabu_search.cpp)
add_library(libkaffpa2 OBJECT ${LIBKAFFPA_SOURCE_FILES})



include_directories(
        app
        lib
        lib/data_structure
        lib/data_structure/priority_queues
        lib/mis
        lib/mis/ils
        lib/mis/initial_mis
        lib/mis/kernel
        lib/mis/ml
        lib/tools
        extern/KaHIP
        extern/KaHIP/app
        extern/KaHIP/interface
        extern/KaHIP/lib
        extern/KaHIP/lib/algorithms
        extern/KaHIP/lib/data_structure
        extern/KaHIP/lib/data_structure/matrix
        extern/KaHIP/lib/data_structure/priority_queues
        extern/KaHIP/lib/io
        extern/KaHIP/lib/parallel_mh
        extern/KaHIP/lib/parallel_mh/exchange
        extern/KaHIP/lib/parallel_mh/galinier_combine
        extern/KaHIP/lib/partition
        extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement
        extern/KaHIP/lib/tools

        extern/frugally-deep/include
        extern/eigen/
        extern/FunctionalPlus/include
        extern/FunctionalPlus/include_all_in_one/include/
        extern/xgboost/include
        extern/json/include
        extern/json/single_include
        extern/algo_test/
)


set(SOURCES
        lib/tools/mis_log.cpp
        lib/mis/ils/ils.cpp
        lib/mis/ils/local_search.cpp
        lib/mis/initial_mis/greedy_mis.cpp
        lib/mis/initial_mis/greedy_vertex.cpp
        lib/mis/initial_mis/random_mis.cpp
        lib/mis/initial_mis/initial_mis.cpp
        lib/data_structure/mis_permutation.cpp
        lib/data_structure/candidate_list.cpp
        lib/data_structure/operation_log.cpp
        lib/data_structure/priority_queues/bucket_array.cpp
        lib/mis/kernel/branch_and_reduce_algorithm.cpp
        lib/mis/kernel/reductions.cpp
        ../extern/argtable3-3.0.3/argtable3.c
        )

add_library(libsources OBJECT ${SOURCES})

set(ML_SOURCES
        lib/mis/ml/ml_features.cpp
        lib/mis/ml/weighted_ls.cpp
        lib/tools/graph_builder.cpp
        lib/tools/synthetic_graph_builder.cpp
        lib/tools/util.cpp
        )

add_subdirectory(extern/xgboost)
add_subdirectory(extern/json)
add_subdirectory(extern/docopt.cpp)

add_library(libml OBJECT ${ML_SOURCES} lib/tools/graph_builder.h)
target_link_libraries(libml nlohmann_json::nlohmann_json)


add_executable(branch_reduce "app/branch_reduce.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(branch_reduce xgboost)

add_executable(weighted_ls "app/weighted_ls.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(weighted_ls xgboost)

add_executable(merge_graph_weights "app/merge_graph_weights.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(merge_graph_weights xgboost)

add_executable(train_model "app/train_model.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(train_model xgboost)

add_executable(iterative_ml "app/iterative_ml.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(iterative_ml xgboost)

add_executable(greedy_reduction "app/greedy_reduction.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(greedy_reduction xgboost)

add_executable(analyze "app/analyze.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml> app/synthetic_config.h)
target_link_libraries(analyze xgboost)

add_executable(assign_weights "app/assign_weights.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(assign_weights xgboost)

add_executable(read_IS "app/read_IS.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(read_IS xgboost)

add_executable(simple_greedy "app/simple_greedy.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(simple_greedy xgboost)

add_executable(simple_ml "app/simple_ml.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(simple_ml xgboost)

add_executable(metis_to_dot "app/metis_to_dot.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(metis_to_dot xgboost)

add_executable(gen_synthetics "app/gen_synthetics.cpp" $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(gen_synthetics xgboost)

add_executable(simple_nn "app/simple_nn.cpp"  $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libml>)
target_link_libraries(simple_nn xgboost)