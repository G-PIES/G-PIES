#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "cluster_dynamics_metal_impl.hpp"

#include <stdio.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <cstring>

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  SIMULATION CONTROL FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

bool ClusterDynamicsImpl::step(gp_float delta_time) {
  mtl_kernel.step_init();
  mtl_kernel.update_clusters_1(delta_time);

  // CPU memory to GPU memory
  gp_float* interstitials_in = (gp_float*)mtl_interstitials_in->contents();
  gp_float* vacancies_in = (gp_float*)mtl_vacancies_in->contents();

  // copy from index 1 to concentration_boundary - 1
  memcpy(interstitials_in + 1, mtl_kernel.interstitials + 1,
         sizeof(gp_float) * concentration_boundary);
  memcpy(vacancies_in + 1, mtl_kernel.vacancies + 1,
         sizeof(gp_float) * concentration_boundary);

  mtl_update_clusters(delta_time);

  // GPU memory to CPU memory
  // copy from index 2 to concentration_boundary - 1
  gp_float* interstitials_out = (gp_float*)mtl_interstitials_out->contents();
  gp_float* vacancies_out = (gp_float*)mtl_vacancies_out->contents();

  /* NOTE:
   *   We are ignoring the size 2 interstitial cluster calculation. Size 2
   *   interstitial clusters are represented as 2 size 1 clusters.
   *   This is to address the instability of interstitial migration in a
   *   size 2 cluster which can be problematic for the simulation.
   */
  memcpy(mtl_kernel.interstitials + 3, interstitials_out + 3,
         sizeof(gp_float) * (concentration_boundary - 1));
  mtl_kernel.interstitials[2] +=
      mtl_kernel.i_concentration_derivative(2) * delta_time;

  memcpy(mtl_kernel.vacancies + 2, vacancies_out + 2,
         sizeof(gp_float) * (concentration_boundary - 1));

  mtl_kernel.update_dislocation_density(delta_time);

  return true;  // TODO - exception handling
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  PUBLIC INTERFACE FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

// TODO - clean up the uses of random +1/+2/-1/etc throughout the code
ClusterDynamicsImpl::ClusterDynamicsImpl(size_t concentration_boundary,
                                         const NuclearReactorImpl& reactor,
                                         const MaterialImpl& material)
    : time(0.0), concentration_boundary(concentration_boundary) {
  mtl_init_kernel(reactor, material);
  mtl_init_lib();
  mtl_init_buffers();
}

ClusterDynamicsImpl::~ClusterDynamicsImpl() {
  mtl_ar_pool->release();
  delete[] mtl_kernel.interstitials;
  delete[] mtl_kernel.vacancies;
}

ClusterDynamicsState ClusterDynamicsImpl::run(gp_float delta_time,
                                              gp_float total_time) {
  bool state_is_valid = true;

  for (gp_float endtime = time + total_time; time < endtime;
       time += delta_time) {
    state_is_valid = step(delta_time);
    if (!state_is_valid) break;
  }

  return ClusterDynamicsState{
      .valid = state_is_valid,
      .time = time,
      .interstitials = std::vector<gp_float>(
          mtl_kernel.interstitials,
          mtl_kernel.interstitials + concentration_boundary),
      .vacancies = std::vector<gp_float>(
          mtl_kernel.vacancies, mtl_kernel.vacancies + concentration_boundary),
      .dislocation_density = mtl_kernel.dislocation_density};
}

MaterialImpl ClusterDynamicsImpl::get_material() { return mtl_kernel.material; }

void ClusterDynamicsImpl::set_material(const MaterialImpl& material) {
  mtl_kernel.material = material;
}

NuclearReactorImpl ClusterDynamicsImpl::get_reactor() {
  return mtl_kernel.reactor;
}

void ClusterDynamicsImpl::set_reactor(const NuclearReactorImpl& reactor) {
  mtl_kernel.reactor = reactor;
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  METAL SHADER ROUTINES
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

void ClusterDynamicsImpl::mtl_init_kernel(const NuclearReactorImpl& reactor,
                                          const MaterialImpl& material) {
  mtl_kernel.interstitials = new gp_float[concentration_boundary + 1];
  mtl_kernel.vacancies = new gp_float[concentration_boundary + 1];
  mtl_kernel.reactor = reactor;
  mtl_kernel.material = material;
  mtl_kernel.concentration_boundary = concentration_boundary;
  mtl_kernel.dislocation_density = material.dislocation_density_0;
}

void ClusterDynamicsImpl::mtl_init_lib() {
  mtl_ar_pool = NS::AutoreleasePool::alloc()->init();
  mtl_device = MTL::CreateSystemDefaultDevice();

  NS::String* metallib_path =
      NS::String::string(METALLIB_PATH, NS::UTF8StringEncoding);
  NS::Error* mtl_error;

  MTL::Library* mtl_lib = mtl_device->newLibrary(metallib_path, &mtl_error);
  if (!mtl_lib) fprintf(stderr, "metal_library null\n");

  NS::String* mtl_func_name =
      NS::String::string("update_clusters", NS::ASCIIStringEncoding);
  MTL::Function* mtl_func = mtl_lib->newFunction(mtl_func_name);
  if (!mtl_func) fprintf(stderr, "metal_function null\n");

  mtl_compute_pipeline_state =
      mtl_device->newComputePipelineState(mtl_func, &mtl_error);
  if (!mtl_compute_pipeline_state)
    fprintf(stderr, "metal_compute_pipeline_state null\n");

  mtl_command_queue = mtl_device->newCommandQueue();
  if (!mtl_command_queue) fprintf(stderr, "metal_command_queue null\n");
}

void ClusterDynamicsImpl::mtl_init_buffers() {
  size_t mtl_buf_size = concentration_boundary * sizeof(gp_float);

  mtl_interstitials_in =
      mtl_device->newBuffer(mtl_buf_size, MTL::ResourceStorageModeShared);
  mtl_vacancies_in =
      mtl_device->newBuffer(mtl_buf_size, MTL::ResourceStorageModeShared);
  mtl_interstitials_out =
      mtl_device->newBuffer(mtl_buf_size, MTL::ResourceStorageModeShared);
  mtl_vacancies_out =
      mtl_device->newBuffer(mtl_buf_size, MTL::ResourceStorageModeShared);
}

void ClusterDynamicsImpl::mtl_update_clusters(gp_float delta_time) {
  // create a command buffer to hold commands
  MTL::CommandBuffer* mtl_command_buffer = mtl_command_queue->commandBuffer();
  assert(mtl_command_buffer != nullptr);

  // start a compute pass
  MTL::ComputeCommandEncoder* mtl_compute_encoder =
      mtl_command_buffer->computeCommandEncoder();
  assert(mtl_compute_encoder != nullptr);

  mtl_encode_command(mtl_compute_encoder, delta_time);

  // end the compute pass
  mtl_compute_encoder->endEncoding();

  // execute the command
  mtl_command_buffer->commit();

  mtl_command_buffer->waitUntilCompleted();
}

void ClusterDynamicsImpl::mtl_encode_command(
    MTL::ComputeCommandEncoder* mtl_compute_encoder, gp_float delta_time) {
  // encode the pipeline state object and its parameters
  mtl_compute_encoder->setComputePipelineState(mtl_compute_pipeline_state);

  mtl_compute_encoder->setBytes(&mtl_kernel, sizeof(ClusterDynamicsMetalKernel),
                                0);
  mtl_compute_encoder->setBuffer(mtl_interstitials_in, 0, 1);
  mtl_compute_encoder->setBuffer(mtl_vacancies_in, 0, 2);
  mtl_compute_encoder->setBuffer(mtl_interstitials_out, 0, 3);
  mtl_compute_encoder->setBuffer(mtl_vacancies_out, 0, 4);
  mtl_compute_encoder->setBytes(&delta_time, sizeof(gp_float), 5);

  MTL::Size mtl_grid_size = MTL::Size(concentration_boundary, 1, 1);

  // calculate a threadgroup size
  NS::UInteger mtl_max_threads_per_group =
      mtl_compute_pipeline_state->maxTotalThreadsPerThreadgroup();
  if (mtl_max_threads_per_group > concentration_boundary) {
    mtl_max_threads_per_group = concentration_boundary;
  }

  MTL::Size mtl_thread_group_size = MTL::Size(mtl_max_threads_per_group, 1, 1);

  // encode the compute command
  mtl_compute_encoder->dispatchThreads(mtl_grid_size, mtl_thread_group_size);
}
