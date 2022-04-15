//
// Created by joseph on 4/15/22.
//

#ifndef KAMIS_FEATURE_SET_MACROS_H
#define KAMIS_FEATURE_SET_MACROS_H

#define LARGE 0
#define GREEDY 1
#define WITH_LS 2

/*
#define FEATURE_SET GREEDY
#define FEATURE_SET LARGE
#define FEATURE_SET WITH_LS
 */

#ifndef FEATURE_SET
#define FEATURE_SET LARGE
#endif

#if FEATURE_SET == LARGE
#define F_NODES
#define F_EDGES
#define F_DEG
#define F_CHI2_DEG
#define F_AVG_CHI2_DEG
#define F_LCC
#define F_CHI2_LCC
#define F_CHROMATIC
#define F_T_WEIGHT
#define F_NODE_W
#define F_W_DEG
#define F_CHI2_W_DEG
#define F_MAX_W
#define F_MIN_W
#define F_MAX_NEIGHBORHOOD_W
#define F_HT
#endif

#if FEATURE_SET == GREEDY
#define F_NODES
#define F_EDGES
#define F_DEG
#define F_T_WEIGHT
#define F_NODE_W
#define F_W_DEG
#define F_HT
#endif

#if FEATURE_SET == WITH_LS
#define F_NODES
#define F_EDGES
#define F_DEG
#define F_CHI2_DEG
#define F_AVG_CHI2_DEG
#define F_LCC
#define F_CHI2_LCC
#define F_CHROMATIC
#define F_T_WEIGHT
#define F_NODE_W
#define F_W_DEG
#define F_CHI2_W_DEG
#define F_MAX_W
#define F_MIN_W
#define F_MAX_NEIGHBORHOOD_W
#define F_HT
#define F_LS
#endif


// dependencies
#ifdef F_CHI2_W_DEG
#define F_W_DEG
#endif
#ifdef F_HT
#define F_W_DEG
#endif
#ifdef F_AVG_CHI2_DEG
#define F_CHI2_DEG
#endif
#ifdef F_CHI2_DEG
#define F_DEG
#endif
#ifdef F_CHI2_LCC
#define F_LCC
#endif

#endif //KAMIS_FEATURE_SET_MACROS_H
