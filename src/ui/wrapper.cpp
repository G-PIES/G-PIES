// Need to have pybind11 installed with "pip install pybind11"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <array>
#include <boost/program_options.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/consumers/yaml_consumer.hpp"
#include "utils/timer.hpp"

namespace py = pybind11;

struct Sim_Reactor {
  Sim_Reactor() {
    nuclear_reactors::OSIRIS(reactor);  // For now..
  }

  NuclearReactor get_reactor() { return reactor; }

  void set_flux(const gp_float val) { reactor.set_flux(val); }
  gp_float get_flux() { return reactor.get_flux(); }

  void set_temperature(const gp_float val) { reactor.set_temperature(val); }
  gp_float get_temperature() { return reactor.get_temperature(); }

  void set_recombination(const gp_float val) { reactor.set_recombination(val); }
  gp_float get_recombination() { return reactor.get_recombination(); }

  void set_i_bi(const gp_float val) { reactor.set_i_bi(val); }
  gp_float get_i_bi() { return reactor.get_i_bi(); }

  void set_i_tri(const gp_float val) { reactor.set_i_tri(val); }
  gp_float get_i_tri() { return reactor.get_i_tri(); }

  void set_i_quad(const gp_float val) { reactor.set_i_quad(val); }
  gp_float get_i_quad() { return reactor.get_i_quad(); }

  void set_v_bi(const gp_float val) { reactor.set_v_bi(val); }
  gp_float get_v_bi() { return reactor.get_v_bi(); }

  void set_v_tri(const gp_float val) { reactor.set_v_tri(val); }
  gp_float get_v_tri() { return reactor.get_v_tri(); }

  void set_v_quad(const gp_float val) { reactor.set_v_quad(val); }
  gp_float get_v_quad() { return reactor.get_v_quad(); }

  void set_dislocation_density_evolution(const gp_float val) {
    reactor.set_dislocation_density_evolution(val);
  }
  gp_float get_dislocation_density_evolution() {
    return reactor.get_dislocation_density_evolution();
  }

  NuclearReactor reactor;
};

struct Sim_Material {
  Sim_Material() {
    materials::SA304(material);  // For now..
  }

  Material get_material() { return material; }

  void set_i_migration(const gp_float val) { material.set_i_migration(val); }
  gp_float get_i_migration() { return material.get_i_migration(); }

  void set_v_migration(const gp_float val) { material.set_v_migration(val); }
  gp_float get_v_migration() { return material.get_v_migration(); }

  void set_i_diffusion_0(const gp_float val) {
    material.set_i_diffusion_0(val);
  }
  gp_float get_i_diffusion_0() { return material.get_i_diffusion_0(); }

  void set_v_diffusion_0(const gp_float val) {
    material.set_v_diffusion_0(val);
  }
  gp_float get_v_diffusion_0() { return material.get_v_diffusion_0(); }

  void set_i_formation(const gp_float val) { material.set_i_formation(val); }
  gp_float get_i_formation() { return material.get_i_formation(); }

  void set_v_formation(const gp_float val) { material.set_v_formation(val); }
  gp_float get_v_formation() { return material.get_v_formation(); }

  void set_i_binding(const gp_float val) { material.set_i_binding(val); }
  gp_float get_i_binding() { return material.get_i_binding(); }

  void set_v_binding(const gp_float val) { material.set_v_binding(val); }
  gp_float get_v_binding() { return material.get_v_binding(); }

  void set_recombination_radius(const gp_float val) {
    material.set_recombination_radius(val);
  }
  gp_float get_recombination_radius() {
    return material.get_recombination_radius();
  }

  void set_i_loop_bias(const gp_float val) { material.set_i_loop_bias(val); }
  gp_float get_i_loop_bias() { return material.get_i_loop_bias(); }

  void set_i_dislocation_bias(const gp_float val) {
    material.set_i_dislocation_bias(val);
  }
  gp_float get_i_dislocation_bias() {
    return material.get_i_dislocation_bias();
  }

  void set_i_dislocation_bias_param(const gp_float val) {
    material.set_i_dislocation_bias_param(val);
  }
  gp_float get_i_dislocation_bias_param() {
    return material.get_i_dislocation_bias_param();
  }

  void set_v_loop_bias(const gp_float val) { material.set_v_loop_bias(val); }
  gp_float get_v_loop_bias() { return material.get_v_loop_bias(); }

  void set_v_dislocation_bias(const gp_float val) {
    material.set_v_dislocation_bias(val);
  }
  gp_float get_v_dislocation_bias() {
    return material.get_v_dislocation_bias();
  }

  void set_v_dislocation_bias_param(const gp_float val) {
    material.set_v_dislocation_bias_param(val);
  }
  gp_float get_v_dislocation_bias_param() {
    return material.get_v_dislocation_bias_param();
  }

  void set_dislocation_density_0(const gp_float val) {
    material.set_dislocation_density_0(val);
  }
  gp_float get_dislocation_density_0() {
    return material.get_dislocation_density_0();
  }

  void set_grain_size(const gp_float val) { material.set_grain_size(val); }
  gp_float get_grain_size() { return material.get_grain_size(); }

  void set_lattice_param(const gp_float val) {
    material.set_lattice_param(val);
  }
  gp_float get_lattice_param() { return material.get_lattice_param(); }

  void set_burgers_vector(const gp_float val) {
    material.set_burgers_vector(val);
  }
  gp_float get_burgers_vector() { return material.get_burgers_vector(); }

  void set_atomic_volume(const gp_float val) {
    material.set_atomic_volume(val);
  }
  gp_float get_atomic_volume() { return material.get_atomic_volume(); }

  Material material;
};

// C++ wrapper file for the Python implementation of cluster dyanamics, based
// off of the example C++ cluster dynamics files.
struct Simulation {
  Simulation(std::string config_name)
      : config_name(config_name) {
    /*
    config.data_validation_on = true;
    config.max_cluster_size = 1001;
    config.relative_tolerance = 1.0e-6;
    config.absolute_tolerance = 1.0e+1;
    config.max_num_integration_steps = 5000;
    config.min_integration_step = 1.0e-30;
    config.max_integration_step = 1.0e+20;

    config.reactor = reactor.get_reactor();
    config.material = material.get_material();
    */
    YamlConsumer yaml_consumer(config_name);
    yaml_consumer.populate_cd_config(config);
    config.init_interstitials = std::vector<gp_float>(config.max_cluster_size, 0.);
    config.init_vacancies = std::vector<gp_float>(config.max_cluster_size, 0.);
    cd = std::make_unique<ClusterDynamics>(config);
    std::cout << "Config settings:" << std::endl;
    std::cout << " [DEBUG] time_delta: " << config.time_delta << std::endl;
    std::cout << " [DEBUG] max_cluster_size: " << config.max_cluster_size << std::endl;
    std::cout << " [DEBUG] relative_tolerance: " << config.relative_tolerance << std::endl;
    std::cout << " [DEBUG] max_num_integration_steps: " << config.max_num_integration_steps << std::endl;
    std::cout << " [DEBUG] min_integration_step: " << config.min_integration_step << std::endl;
    std::cout << " [DEBUG] max_integration_step: " << config.max_integration_step << std::endl;
    std::cout << " [DEBUG] flux: " << config.reactor.get_flux() << std::endl;
    std::cout << " [DEBUG] material eV: " << config.material.get_i_migration() << std::endl;
  }

  void run() {
    state = (*cd).run(config.time_delta, config.sample_interval); 
  }

  double get_state_time() { return state.time; }

  double get_simulation_time() { return config.simulation_time; }

  double get_int_idx(int i) { return state.interstitials[i]; }

  double get_vac_idx(int i) { return state.vacancies[i]; }

  std::string config_name;

  //double sample_interval = delta_time;

  std::unique_ptr<ClusterDynamics> cd;

  Sim_Reactor main_sim_reactor;
  Sim_Material main_sim_material;
  ClusterDynamicsState state;
  ClusterDynamicsConfig config;
};

// Binds the current backend Cluster Dynamics code with these wrapper functions
PYBIND11_MODULE(libpyclusterdynamics, m) {
  // Description
  m.doc() = "Cluster Dynamics C++ backend interface for python3.";
  // The name of the python class object is Simulation
  py::class_<Simulation>(m, "Simulation")
      .def(py::init<std::string>())
      .def("run", &Simulation::run)
      .def("get_state_time", &Simulation::get_state_time)
      .def("get_simulation_time", &Simulation::get_simulation_time)
      .def("get_int_idx", &Simulation::get_int_idx)
      .def("get_vac_idx", &Simulation::get_vac_idx);

  py::class_<Sim_Reactor>(m, "Sim_Reactor")
      .def(py::init<>())
      .def("set_flux", &Sim_Reactor::set_flux)
      .def("get_flux", &Sim_Reactor::get_flux)

      .def("set_temperature", &Sim_Reactor::set_temperature)
      .def("get_temperature", &Sim_Reactor::get_temperature)

      .def("set_recombination", &Sim_Reactor::set_recombination)
      .def("get_recombination", &Sim_Reactor::get_recombination)

      .def("set_i_bi", &Sim_Reactor::set_i_bi)
      .def("get_i_bi", &Sim_Reactor::get_i_bi)

      .def("set_i_tri", &Sim_Reactor::set_i_tri)
      .def("get_i_tri", &Sim_Reactor::get_i_tri)

      .def("set_i_quad", &Sim_Reactor::set_i_quad)
      .def("get_i_quad", &Sim_Reactor::get_i_quad)

      .def("set_v_bi", &Sim_Reactor::set_v_bi)
      .def("get_v_bi", &Sim_Reactor::get_v_bi)

      .def("set_v_tri", &Sim_Reactor::set_v_tri)
      .def("get_v_tri", &Sim_Reactor::get_v_tri)

      .def("set_v_quad", &Sim_Reactor::set_v_quad)
      .def("get_v_quad", &Sim_Reactor::get_v_quad)

      .def("set_dislocation_density_evolution",
           &Sim_Reactor::set_dislocation_density_evolution)
      .def("get_dislocation_density_evolution",
           &Sim_Reactor::get_dislocation_density_evolution);

  py::class_<Sim_Material>(m, "Sim_Material")
      .def(py::init<>())
      .def("set_i_migration", &Sim_Material::set_i_migration)
      .def("get_i_migration", &Sim_Material::get_i_migration)

      .def("set_v_migration", &Sim_Material::set_v_migration)
      .def("get_v_migration", &Sim_Material::get_v_migration)

      .def("set_i_diffusion_0", &Sim_Material::set_i_diffusion_0)
      .def("get_i_diffusion_0", &Sim_Material::get_i_diffusion_0)

      .def("set_v_diffusion_0", &Sim_Material::set_v_diffusion_0)
      .def("get_v_diffusion_0", &Sim_Material::get_v_diffusion_0)

      .def("set_i_formation", &Sim_Material::set_i_formation)
      .def("get_i_formation", &Sim_Material::get_i_formation)

      .def("set_v_formation", &Sim_Material::set_v_formation)
      .def("get_v_formation", &Sim_Material::get_v_formation)

      .def("set_i_binding", &Sim_Material::set_i_binding)
      .def("get_i_binding", &Sim_Material::get_i_binding)

      .def("set_recombination_radius", &Sim_Material::set_recombination_radius)
      .def("get_recombination_radius", &Sim_Material::get_recombination_radius)

      .def("set_i_loop_bias", &Sim_Material::set_i_loop_bias)
      .def("get_i_loop_bias", &Sim_Material::get_i_loop_bias)

      .def("set_i_dislocation_bias", &Sim_Material::set_i_dislocation_bias)
      .def("get_i_dislocation_bias", &Sim_Material::get_i_dislocation_bias)

      .def("set_i_dislocation_bias_param",
           &Sim_Material::set_i_dislocation_bias_param)
      .def("get_i_dislocation_bias_param",
           &Sim_Material::get_i_dislocation_bias_param)

      .def("set_v_loop_bias", &Sim_Material::set_v_loop_bias)
      .def("get_v_loop_bias", &Sim_Material::get_v_loop_bias)

      .def("set_v_dislocation_bias", &Sim_Material::set_v_dislocation_bias)
      .def("get_v_dislocation_bias", &Sim_Material::get_v_dislocation_bias)

      .def("set_v_dislocation_bias_param",
           &Sim_Material::set_v_dislocation_bias_param)
      .def("get_v_dislocation_bias_param",
           &Sim_Material::get_v_dislocation_bias_param)

      .def("set_dislocation_density_0",
           &Sim_Material::set_dislocation_density_0)
      .def("get_dislocation_density_0",
           &Sim_Material::get_dislocation_density_0)

      .def("set_grain_size", &Sim_Material::set_grain_size)
      .def("get_grain_size", &Sim_Material::get_grain_size)

      .def("set_lattice_param", &Sim_Material::set_lattice_param)
      .def("get_lattice_param", &Sim_Material::get_lattice_param)

      .def("set_burgers_vector", &Sim_Material::set_burgers_vector)
      .def("get_burgers_vector", &Sim_Material::get_burgers_vector)

      .def("set_atomic_volume", &Sim_Material::set_atomic_volume)
      .def("get_atomic_volume", &Sim_Material::get_atomic_volume);
}
