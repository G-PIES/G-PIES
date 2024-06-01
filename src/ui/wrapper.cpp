// Need to have pybind11 installed with "pip install pybind11" or can be installed with the CMake build system when built.
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Required by the simulation.
#include <array>
#include <boost/program_options.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

// Include all the required libraries that exist within the gpies.cpp file. (running a full simulation.)
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/consumers/yaml_consumer.hpp"
#include "utils/timer.hpp"
#include "utils/progress_bar.hpp"

// Create the pybind11 namespace.
namespace py = pybind11;

// C++ Sim Reactor struct to store the constructor and functions that will be converted into the python
// classes

struct Sim_Reactor {
  Sim_Reactor() {
    nuclear_reactors::OSIRIS(reactor);  // For now.. Default reactor. Future plans to implement mysql implementation
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

// C++ Sim Material struct to store the constructor and functions that will be converted into the python
// classes 
struct Sim_Material {
  Sim_Material() {
    materials::SA304(material);  // For now.. Default material. Future plans to implement mysql implementation
  }

  // Getters and setters for material properties, possibly out of date considering the new config yaml code.
  // Kept in the file as the UI python code still rely on a few of these function. Same for the getters and setters
  // in the reactor class.

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
  gp_float get_atomic_volume() { return material.get_atomic_volume(); }

  Material material;
};

// C++ wrapper file for the Python implementation of cluster dyanamics, based
// off of the example C++ cluster dynamics files.
struct Simulation {
  Simulation(std::string config_name)
      : config_name(config_name) {
    // Load the YAML config via a name provided, absolute path of the config file.
    YamlConsumer yaml_consumer(config_name);
    yaml_consumer.populate_cd_config(config);
    // initialize the cluster interstials and vacancies to the default amounts.
    config.init_interstitials = std::vector<gp_float>(config.max_cluster_size, 0.);
    config.init_vacancies = std::vector<gp_float>(config.max_cluster_size, 0.);
    
  }

  void run() {
    // Creates a new ClusterDynamics with CPU capabilities
    ClusterDynamics cd = ClusterDynamics::cpu(config);
    // Deference ClusterDynamics and call the run function, useful if the user wants to define 
    // their own simulation in Python instead of using the below `run_full_simulation` function.
    state = cd.run(config.time_delta, config.sample_interval); 
  }

  // Copy of the main simulation loop function in the gpies.cpp file.
  void run_full_simulation(bool print_step, bool create_csv) {
    // Creates a new ClusterDynamics with CPU capabilities
    ClusterDynamics cd = ClusterDynamics::cpu(config);

    std::cout << "ClusterDynamics Python Simulation:" << std::endl;
    std::cout << "(Simulation may take a while to initialize...)" << std::endl;
    
    progressbar bar(
      static_cast<int>(config.simulation_time / config.time_delta), true,
      std::cout);
    bar.set_todo_char(" ");
    bar.set_done_char("█");
    bar.set_opening_bracket_char("[");
    bar.set_closing_bracket_char("]");

    for (gp_float t = 0.; t < config.simulation_time; t = state.time) {
      if (!print_step) {
        bar.update();
      }

      state = cd.run(config.time_delta, config.sample_interval);
      //Prints the state at each interval
      if (print_step) {
        print_state();
      }
    }
    if (!create_csv) {
      std::cout << "Results:" << std::endl;
      //stdout Result
      print_state();
    }
    else {
      write_csv();
      std::cout << "\nWrote results to \'./out.csv\'." << std::endl;
    }
  }

  void write_csv() {
    std::ofstream csv;
    csv.open("./out.csv");
    csv << state.time << ", " << state.dislocation_density;
    for (uint64_t n = 1; n < config.max_cluster_size; ++n) {
      csv << "," << state.interstitials[n] << "," << state.vacancies[n];
    }
    csv << std::endl;
  }

  // Copy of the function that exists in the gpies.cpp file. Just prints the current state of the sim.
  void print_state() {
    std::cout << "\nTime=" << state.time;
    size_t size = state.interstitials.size();
    std::cout << "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n";
    for (size_t n = 1; n < size; ++n) {
      for (size_t n = 1; n < size; ++n) {
        std::cout << (long long unsigned int)n << "\t\t\t\t\t" << std::setprecision(13)
          << state.interstitials[n] << "\t\t\t" << std::setprecision(15)
          << state.vacancies[n] << std::endl;
      }

      std::cout << "\nDislocation Network Density: " << state.dislocation_density
        << std::endl;
    }
  }

  // Getters and setters useful for the UI implementation. Getting simulation values can be 
  // achieved with similar function in the future.

  double get_state_time() { return state.time; }

  double get_simulation_time() { return config.simulation_time; }

  double get_int_idx(int i) { return state.interstitials[i]; }

  double get_vac_idx(int i) { return state.vacancies[i]; }

  // Name of the configuration in the absolute path.
  std::string config_name;

  //double sample_interval = delta_time;

  // The public varaibles for the current simulations reactor and material types. As well as the state and config.
  // Used to access the values with getters and setters.
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
      .def("run_full_simulation", &Simulation::run_full_simulation)
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
           &Sim_Material::get_dislocation_density_0);

      //.def("set_grain_size", &Sim_Material::set_grain_size)
      //.def("get_grain_size", &Sim_Material::get_grain_size)

      //.def("set_lattice_param", &Sim_Material::set_lattice_param)
      //.def("get_lattice_param", &Sim_Material::get_lattice_param)

      //.def("set_burgers_vector", &Sim_Material::set_burgers_vector)
      //.def("get_burgers_vector", &Sim_Material::get_burgers_vector)

      //.def("set_atomic_volume", &Sim_Material::set_atomic_volume)
      //.def("get_atomic_volume", &Sim_Material::get_atomic_volume);
}
