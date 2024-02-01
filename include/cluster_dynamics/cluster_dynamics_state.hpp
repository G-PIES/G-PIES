#pragma once

#include <vector>

struct ClusterDynamicsState
{
    bool valid = true;
    double time = 0.0;
    std::vector<std::vector<double>> interstitials;
    std::vector<std::vector<double>> vacancies;
    double dislocation_density = 0.0;
};
