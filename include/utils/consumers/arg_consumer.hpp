#ifndef ARG_CONSUMER
#define ARG_CONSUMER

#include <string>
#include <vector>

#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "utils/gpies_exception.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/types.hpp"

class ArgConsumer {
 public:
  virtual bool has_arg(const std::string &, const std::string & = "") = 0;
  virtual std::string get_string(const std::string &,
                                 const std::string & = "") = 0;
  virtual size_t get_size_t(const std::string &, const std::string & = "") = 0;
  virtual gp_float get_float(const std::string &, const std::string & = "") = 0;
  virtual void populate_init_interstitials(ClusterDynamicsConfig &) = 0;
  virtual void populate_init_vacancies(ClusterDynamicsConfig &) = 0;

  void populate_cd_config(ClusterDynamicsConfig &cd_config) {
    if (has_arg("time", "simulation")) {
      gp_float st = get_float("time", "simulation");
      if (st <= 0.)
        throw GpiesException(
            "Value for time must be a positive, non-zero decimal.");

      cd_config.simulation_time = st;
    }

    if (has_arg("time-delta", "simulation")) {
      gp_float td = get_float("time-delta", "simulation");
      if (td <= 0.)
        throw GpiesException(
            "Value for time-delta must be a positive, non-zero "
            "decimal.");

      cd_config.time_delta = td;
    }

    if (has_arg("sample-interval", "simulation")) {
      gp_float si = get_float("sample-interval", "simulation");
      if (si <= 0.)
        throw GpiesException(
            "Value for sample-interval must be a positive, non-zero "
            "decimal.");

      cd_config.sample_interval = si;
    }

    // Toggle data validation
    if (has_arg("data-validation", "simulation")) {
      cd_config.data_validation_on =
          0 == get_string("data-validation", "simulation").compare("on");
    }

    if (has_arg("max-cluster-size", "simulation")) {
      size_t mcs = get_size_t("max-cluster-size", "simulation");
      if (mcs <= 0)
        throw GpiesException(
            "Value for max-cluster-size must be a positive, non-zero integer.");

      cd_config.max_cluster_size = mcs;
    }

    if (has_arg("relative-tolerance", "simulation")) {
      gp_float rt = get_float("relative-tolerance", "simulation");
      if (rt <= 0.)
        throw GpiesException(
            "Value for relative-tolerance must be a positive, non-zero "
            "decimal.");

      cd_config.relative_tolerance = rt;
    }

    if (has_arg("absolute-tolerance", "simulation")) {
      gp_float at = get_float("absolute-tolerance", "simulation");
      if (at <= 0.)
        throw GpiesException(
            "Value for absolute-tolerance must be a positive, non-zero "
            "decimal.");

      cd_config.absolute_tolerance = at;
    }

    if (has_arg("max-num-integration-steps", "simulation")) {
      size_t maxnis = get_float("max-num-integration-steps", "simulation");
      if (maxnis <= 0)
        throw GpiesException(
            "Value for max-num-integration-steps must be a positive, non-zero "
            "integer.");

      cd_config.max_num_integration_steps = maxnis;
    }

    if (has_arg("min-integration-step", "simulation")) {
      gp_float minnis = get_float("min-integration-step", "simulation");
      if (minnis <= 0.)
        throw GpiesException(
            "Value for min-integration-step must be a positive, non-zero "
            "decimal.");

      cd_config.min_integration_step = minnis;
    }

    if (has_arg("max-integration-step", "simulation")) {
      gp_float maxis = get_float("max-integration-step", "simulation");
      if (maxis <= 0.)
        throw GpiesException(
            "Value for max-integration-step must be a positive, non-zero "
            "decimal.");

      cd_config.max_integration_step = maxis;
    }

    if (has_arg("reactor")) {
      populate_reactor(cd_config.reactor);
    } else {
      nuclear_reactors::OSIRIS(cd_config.reactor);
    }

    if (has_arg("material")) {
      populate_material(cd_config.material);
    } else {
      materials::SA304(cd_config.material);
    }

    if (has_arg("init-interstitials")) {
      populate_init_interstitials(cd_config);
    } else {
      cd_config.init_interstitials =
          std::vector<gp_float>(cd_config.max_cluster_size, 0.);
    }

    if (has_arg("init-vacancies")) {
      populate_init_vacancies(cd_config);
    } else {
      cd_config.init_vacancies =
          std::vector<gp_float>(cd_config.max_cluster_size, 0.);
    }
  }

  void populate_reactor(NuclearReactor &reactor) {
    if (has_arg("reactor")) {
      reactor.set_flux(get_float("flux-dpa-s", "reactor"));
      reactor.set_temperature(get_float("temperature-kelvin", "reactor"));
      reactor.set_recombination(get_float("recombination-rate", "reactor"));
      reactor.set_i_bi(get_float("bi-interstitial-rate", "reactor"));
      reactor.set_i_tri(get_float("tri-interstitial-rate", "reactor"));
      reactor.set_i_quad(get_float("quad-interstitial-rate", "reactor"));
      reactor.set_v_bi(get_float("bi-vacancy-rate", "reactor"));
      reactor.set_v_tri(get_float("tri-vacancy-rate", "reactor"));
      reactor.set_v_quad(get_float("quad-vacancy-rate", "reactor"));
      reactor.set_dislocation_density_evolution(
          get_float("dislocation-density-evolution", "reactor"));
    }
  }

  void populate_material(Material &material) {
    if (has_arg("material")) {
      material.set_i_migration(
          get_float("interstitial-migration-ev", "material"));
      material.set_v_migration(get_float("vacancy-migration-ev", "material"));
      material.set_i_diffusion_0(
          get_float("initial-interstitial-diffusion", "material"));
      material.set_v_diffusion_0(
          get_float("initial-vacancy-diffusion", "material"));
      material.set_i_formation(
          get_float("interstitial-formation-ev", "material"));
      material.set_v_formation(get_float("vacancy-formation-ev", "material"));
      material.set_i_binding(get_float("interstitial-binding-ev", "material"));
      material.set_v_binding(get_float("vacancy-binding-ev", "material"));
      material.set_recombination_radius(
          get_float("recombination-radius-cm", "material"));
      material.set_i_loop_bias(get_float("interstitial-loop-bias", "material"));
      material.set_i_dislocation_bias(
          get_float("interstitial-dislocation-bias", "material"));
      material.set_i_dislocation_bias_param(
          get_float("interstitial-dislocation-bias-param", "material"));
      material.set_v_loop_bias(get_float("vacancy-loop-bias", "material"));
      material.set_v_dislocation_bias(
          get_float("vacancy-dislocation-bias", "material"));
      material.set_v_dislocation_bias_param(
          get_float("vacancy-dislocation-bias-param", "material"));
      material.set_dislocation_density_0(
          get_float("initial-dislocation-density-cm^-2", "material"));
      material.set_grain_size(get_float("grain-size-cm", "material"));
      material.set_lattice_param(get_float("lattice-param-cm", "material"));
    }
  }

  SensitivityVariable get_sa_var() {
    std::string arg_name =
        get_string("sensitivity-var", "sensitivity-analysis");

    if (sensitivity_variables.count(arg_name)) {
      return sensitivity_variables[arg_name];
    }

    return SensitivityVariable::NONE;
  }
};

#endif  // ARG_CONSUMER
