#ifndef CLUSTER_DYNAMICS_STATE_HPP
#define CLUSTER_DYNAMICS_STATE_HPP

#include "types.hpp"
#include <vector>

struct ClusterDynamicsState
{
    bool valid = true;
    gp_float time = 0.0;
    std::vector<gp_float> interstitials;
    std::vector<gp_float> vacancies;
    gp_float dislocation_density = 0.0;
};

#endif // CLUSTER_DYNAMICS_STATE_HPP