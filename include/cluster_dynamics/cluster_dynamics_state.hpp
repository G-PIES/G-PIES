#pragma once

#include <vector>
#include <exception>

struct ClusterDynamicsState
{
    bool valid = true;
    double time = 0.0;
    std::vector<std::vector<double>> interstitials;
    std::vector<std::vector<double>> vacancies;
    std::vector<double> dislocation_density;
};

class ClusterDynamicsException{
};
