#ifndef CLUSTER_DYNAMICS_EXCEPTION_HPP
#define CLUSTER_DYNAMICS_EXCEPTION_HPP

#include <string>
#include <vector>

#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "utils/gpies_exception.hpp"

class ClusterDynamicsException : public GpiesException {
 public:
  ClusterDynamicsException(const std::string &message,
                    const ClusterDynamicsState &err_state)
      : err_state(err_state), GpiesException(message) {}

  ClusterDynamicsState err_state;
};

#endif  // CLUSTER_DYNAMICS_EXCEPTION_HPP
