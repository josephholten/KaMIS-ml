//
// Created by joseph on 10/14/21.
//

#ifndef MWIS_ML_STAT_H
#define MWIS_ML_STAT_H

#include <algorithm>

template<class T1, class T2>
double chi2(T1 obs, T2 exp) {
    if (exp == 0)
        return 0;
    double diff = (double) obs - (double) exp;
    return (diff * diff) / exp;
}

template<class Number>
double std_dev(std::vector<Number> values) {
    double mean = std::accumulate(values.begin(),  values.end(), 0) / values.size();
    double dev = 0;
    for (const auto& value : values) {
        dev += (value - mean) * (value - mean);
    }
    dev /= (double) values.size();
    return dev;
}

#endif //MWIS_ML_STAT_H
