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
#include "constants.hpp"
#include "nuclear_reactor_impl.hpp"
#include "material_impl.hpp"
#include "cluster_dynamics_metal_args.hpp"

class ClusterDynamicsImpl
{
public:
    gp_float time;

    // GPU
    MTL::Device* mtl_device;

    // metal auto resource management
    NS::AutoreleasePool* mtl_ar_pool;

    // compute pipeline generated from the compute kernel in the .metal shader file
    MTL::ComputePipelineState* mtl_compute_pipeline_state;

    // command queue used to pass commands to the device
    MTL::CommandQueue* mtl_command_queue;

    // metal buffers
    MTL::Buffer* mtl_interstitials_in;
    MTL::Buffer* mtl_vacancies_in;
    MTL::Buffer* mtl_interstitials_out;
    MTL::Buffer* mtl_vacancies_out;

    ClusterDynamicsMetalArgs mtl_args;

    std::vector<gp_float> interstitials;
    std::vector<gp_float> vacancies;

    size_t concentration_boundary;
    gp_float dislocation_density;

    gp_float mean_dislocation_radius_val;
    gp_float ii_sum_absorption_val;
    gp_float iv_sum_absorption_val;
    gp_float vv_sum_absorption_val;
    gp_float vi_sum_absorption_val;
    gp_float i1_val;
    gp_float v1_val;

    MaterialImpl material;
    NuclearReactorImpl reactor;

    gp_float i_defect_production(size_t);
    gp_float v_defect_production(size_t);
    gp_float i_clusters_delta(size_t);
    gp_float v_clusters_delta(size_t);
    gp_float iemission_vabsorption_np1(size_t);
    gp_float vemission_iabsorption_np1(size_t);
    gp_float iemission_vabsorption_n(size_t);
    gp_float vemission_iabsorption_n(size_t);
    gp_float iemission_vabsorption_nm1(size_t);
    gp_float vemission_iabsorption_nm1(size_t);
    gp_float i1_cluster_delta();
    gp_float v1_cluster_delta();
    gp_float i_emission_rate();
    gp_float v_emission_rate();
    gp_float i_absorption_rate();
    gp_float v_absorption_rate();
    gp_float annihilation_rate();
    gp_float i_dislocation_annihilation_rate();
    gp_float v_dislocation_annihilation_rate();
    gp_float i_grain_boundary_annihilation_rate();
    gp_float v_grain_boundary_annihilation_rate();
    gp_float ii_emission(size_t);
    gp_float vv_emission(size_t);
    gp_float ii_absorption(size_t);
    gp_float vi_absorption(size_t);
    gp_float iv_absorption(size_t);
    gp_float vv_absorption(size_t);
    gp_float i_bias_factor(size_t);
    gp_float v_bias_factor(size_t);
    gp_float i_binding_energy(size_t);
    gp_float v_binding_energy(size_t);
    gp_float i_diffusion();
    gp_float v_diffusion();
    gp_float dislocation_promotion_probability(size_t);
    gp_float cluster_radius(size_t);

    // Simulation Operation Functions
    gp_float dislocation_density_delta();
    gp_float mean_dislocation_cell_radius();
    gp_float ii_sum_absorption(size_t);
    gp_float iv_sum_absorption(size_t);
    gp_float vv_sum_absorption(size_t);
    gp_float vi_sum_absorption(size_t);
    void step_init();
    bool step(gp_float);
    bool update_clusters_1(gp_float);
    bool validate(size_t);

    // Interface functions
    ClusterDynamicsImpl(size_t concentration_boundary, const NuclearReactorImpl& reactor, const MaterialImpl& material);
    ~ClusterDynamicsImpl();
        
    ClusterDynamicsState run(gp_float delta_time, gp_float total_time);
    MaterialImpl get_material();
    void set_material(const MaterialImpl& material);
    NuclearReactorImpl get_reactor();
    void set_reactor(const NuclearReactorImpl& reactor);

private:
    // metal shader routines 
    void mtl_init_lib();
    void mtl_init_args();
    void mtl_init_buffers();
    void mtl_send_command();
    void mtl_encode_command(MTL::ComputeCommandEncoder*);
};

#endif // CLUSTER_DYNAMICS_METAL_IMPL_HPP