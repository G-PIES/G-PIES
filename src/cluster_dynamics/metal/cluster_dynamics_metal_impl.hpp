#ifndef CLUSTER_DYNAMICS_METAL_IMPL_HPP
#define CLUSTER_DYNAMICS_METAL_IMPL_HPP

#ifndef METALLIB_PATH 
#define METALLIB_PATH "** absolute path to .metallib **"
#endif

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <vector>

#include "cluster_dynamics_state.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"
#include "cluster_dynamics_metal_args.hpp"

class ClusterDynamicsImpl
{
public:
    float time;

    // GPU
    MTL::Device* mtl_device;

    // metal auto resource management
    NS::AutoreleasePool* mtl_ar_pool;

    // compute pipeline generated from the compute kernel in the .metal shader file
    MTL::ComputePipelineState* mtl_compute_pipeline_state;

    // command queue used to pass commands to the device
    MTL::CommandQueue* mtl_command_queue;

    // metal buffers
    MTL::Buffer* mtl_interstitials_out;
    MTL::Buffer* mtl_vacancies_out;

    ClusterDynamicsMetalArgs mtl_args;

    std::vector<float> interstitials;
    std::vector<float> vacancies;

    size_t concentration_boundary;
    float dislocation_density;

    float mean_dislocation_radius_val;
    float ii_sum_absorption_val;
    float iv_sum_absorption_val;
    float vv_sum_absorption_val;
    float vi_sum_absorption_val;
    float i1_val;
    float v1_val;

    Material material;
    NuclearReactor reactor;

    float i_defect_production(size_t);
    float v_defect_production(size_t);
    float i_clusters_delta(size_t);
    float v_clusters_delta(size_t);
    float iemission_vabsorption_np1(size_t);
    float vemission_iabsorption_np1(size_t);
    float iemission_vabsorption_n(size_t);
    float vemission_iabsorption_n(size_t);
    float iemission_vabsorption_nm1(size_t);
    float vemission_iabsorption_nm1(size_t);
    float i1_cluster_delta();
    float v1_cluster_delta();
    float i_emission_time();
    float v_emission_time();
    float i_absorption_time();
    float v_absorption_time();
    float annihilation_rate();
    float i_dislocation_annihilation_time();
    float v_dislocation_annihilation_time();
    float i_grain_boundary_annihilation_time();
    float v_grain_boundary_annihilation_time();
    float ii_emission(size_t);
    float vv_emission(size_t);
    float ii_absorption(size_t);
    float vi_absorption(size_t);
    float iv_absorption(size_t);
    float vv_absorption(size_t);
    float i_bias_factor(size_t);
    float v_bias_factor(size_t);
    float i_binding_energy(size_t);
    float v_binding_energy(size_t);
    float i_diffusion();
    float v_diffusion();
    float dislocation_promotion_probability(size_t);
    float cluster_radius(size_t);

    // Simulation Operation Functions
    float dislocation_density_delta();
    float mean_dislocation_cell_radius();
    float ii_sum_absorption(size_t);
    float iv_sum_absorption(size_t);
    float vv_sum_absorption(size_t);
    float vi_sum_absorption(size_t);
    void step_init();
    bool step(float);
    bool update_clusters_1(float);
    bool validate(size_t);

    // Interface functions
    ClusterDynamicsImpl(size_t concentration_boundary, const NuclearReactor& reactor, const Material& material);
    ~ClusterDynamicsImpl();
        
    ClusterDynamicsState run(float delta_time, float total_time);
    Material get_material();
    void set_material(const Material& material);
    NuclearReactor get_reactor();
    void set_reactor(const NuclearReactor& reactor);

private:
    // metal shader routines 
    void mtl_init_lib();
    void mtl_init_args();
    void mtl_init_buffers();
    void mtl_send_command();
    void mtl_encode_command(MTL::ComputeCommandEncoder*);
};

#endif // CLUSTER_DYNAMICS_METAL_IMPL_HPP