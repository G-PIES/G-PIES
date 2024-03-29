#ifndef CLUSTER_DYNAMICS_METAL_IMPL_HPP
#define CLUSTER_DYNAMICS_METAL_IMPL_HPP

#ifndef METALLIB_PATH
#define METALLIB_PATH "** absolute path to .metallib **"
#endif

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <vector>

#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "cluster_dynamics_metal_kernel.hpp"
#include "material_impl.hpp"
#include "nuclear_reactor_impl.hpp"
#include "utils/constants.hpp"

class ClusterDynamicsImpl {
 public:
  gp_float time;

  // GPU
  MTL::Device *mtl_device;

  // metal auto resource management
  NS::AutoreleasePool *mtl_ar_pool;

  // compute pipeline generated from the compute kernel in the .metal shader
  // file
  MTL::ComputePipelineState *mtl_compute_pipeline_state;

  // command queue used to pass commands to the device
  MTL::CommandQueue *mtl_command_queue;

  // metal buffers
  MTL::Buffer *mtl_interstitials_in;
  MTL::Buffer *mtl_vacancies_in;
  MTL::Buffer *mtl_interstitials_out;
  MTL::Buffer *mtl_vacancies_out;

  ClusterDynamicsMetalKernel mtl_kernel;

  size_t max_cluster_size;

  bool data_validation_on;

  void step(gp_float);
  void validate_all() const;
  void validate(size_t) const;

  // Interface functions
  ClusterDynamicsImpl(size_t, const NuclearReactorImpl &, const MaterialImpl &);
  ~ClusterDynamicsImpl();

  ClusterDynamicsState run(gp_float time_delta, gp_float total_time);
  MaterialImpl get_material();
  void set_material(const MaterialImpl &material);
  NuclearReactorImpl get_reactor();
  void set_reactor(const NuclearReactorImpl &reactor);

 private:
  // metal shader routines
  void mtl_init_kernel(const NuclearReactorImpl &, const MaterialImpl &);
  void mtl_init_lib();
  void mtl_init_buffers();
  void mtl_update_clusters(gp_float);
  void mtl_encode_command(MTL::ComputeCommandEncoder *, gp_float);
};

#endif  // CLUSTER_DYNAMICS_METAL_IMPL_HPP
