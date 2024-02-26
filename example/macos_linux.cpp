#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>

#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics_impl.hpp"
#include "model/material.hpp"

static constexpr size_t SIM_SIZE = 10;
static constexpr double SIM_DELTA_TIME = 1e-5;
static constexpr double SIM_RUN_TIME = 1e-5;

gp_float func() {
    return 1.7 + (.5 - 1.7) / (std::pow(2., .8) - 1) *
              (std::pow((gp_float)3, .8) - std::pow((gp_float)3 - 1., .8));
}

int main() {
    std::cout << std::setprecision(20);

    NuclearReactor reactor;
    Material material;

    nuclear_reactors::OSIRIS(reactor);
    materials::SA304(material);

    ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
    cd.run(SIM_DELTA_TIME, SIM_RUN_TIME);

    gp_float temperature = CELCIUS_KELVIN_CONV(330.);

    gp_float time = 0.;
    for (size_t vn = 3; vn < 10 - 1; ++vn) {
        gp_float item = 2. * M_PI * std::sqrt(std::sqrt(3.) *
                    std::pow(PM_CM_CONV(360.), 2.) *
                     (gp_float)vn / (4. * M_PI)) *
        // (2)
        cd.v_bias_factor(vn) *
        // (3)
        cd.v_diffusion_val *
        // (4)
        exp(-cd.v_binding_energy(vn) / (BOLTZMANN_EV_KELVIN * temperature)) *
        cd.vacancies[vn];
        std::cout << "item: " << vn << " " << item << std::endl;
        time += item;
    }

    time +=
        // (2)
        4. * cd.vv_emission(2) * cd.vacancies[2]
        // (3)
        + cd.vi_absorption(2) * cd.interstitials[1] * cd.vacancies[2];

    gp_float expected = 2.1585231596640677e-35;
    gp_float actual = cd.v_emission_rate();

    std::cout << "expected: " << expected << std::endl;
    std::cout << "  actual: " << actual << std::endl;
    std::cout << "    time: " << time << std::endl;

    gp_float test = 2. * M_PI * std::sqrt(std::sqrt(3.) *
                    std::pow(PM_CM_CONV(360.), 2.) *
                     (gp_float)3 / (4. * M_PI)) *
        // (2)
        cd.v_bias_factor(3) *
        // (3)
        cd.v_diffusion_val *
        // (4)
        exp(-cd.v_binding_energy(3) / (BOLTZMANN_EV_KELVIN * temperature)) *
        cd.vacancies[3];

    gp_float test2_part1 = 2. * M_PI * std::sqrt(std::sqrt(3.) *
                    std::pow(PM_CM_CONV(360.), 2.) *
                     (gp_float)3 / (4. * M_PI));

    gp_float test2_part2 = cd.v_bias_factor(3);
    gp_float test2_part3 = cd.v_diffusion_val;
    gp_float test2_part4_1 = cd.v_binding_energy(3);
    gp_float test2_part4_2 = BOLTZMANN_EV_KELVIN * temperature;
    gp_float test2_part4 = exp(-test2_part4_1 / test2_part4_2);
    gp_float test2_part5 = cd.vacancies[3];

    gp_float test2 = test2_part1 * test2_part2 * test2_part3 * test2_part4 * test2_part5;

    std::cout << "    test: " << test << std::endl;
    std::cout << "   test2: " << test2 << std::endl;
    std::cout << " test2_1: " << test2_part1 << std::endl;
    std::cout << " test2_2: " << test2_part2 << std::endl;
    std::cout << " test2_3: " << test2_part3 << std::endl;
    std::cout << " test2_4: " << test2_part4 << std::endl;
    std::cout << " test2_5: " << test2_part5 << std::endl;
    std::cout << " test2_4_1: " << test2_part4_1 << std::endl;
    std::cout << " test2_4_2: " << test2_part4_2 << std::endl;

    gp_float test3_1 = .5 - 1.7;
    gp_float test3_2 = std::pow(2., .8) - 1;
    gp_float test3_3 = std::pow((gp_float)3, .8);
    gp_float test3_4 = std::pow((gp_float)3 - 1., .8);
    gp_float test3_5 = (test3_1) / (test3_2) * (test3_3 - test3_4);
    gp_float test3 = 1.7 + test3_5;

    gp_float test4 =  1.7 + (.5 - 1.7) / (std::pow(2., .8) - 1) *
              (std::pow((gp_float)3, .8) - std::pow((gp_float)3 - 1., .8));

    std::cout << " test2_4_1: " << test2_part4_1 << std::endl;
    std::cout << "     test4: " << test4 << std::endl;
    std::cout << "     test3: " << test3 << std::endl;
    std::cout << "   test3_1: " << test3_1 << std::endl;
    std::cout << "   test3_2: " << test3_2 << std::endl;
    std::cout << "   test3_3: " << test3_3 << std::endl;
    std::cout << "   test3_4: " << test3_4 << std::endl;
    std::cout << "   test3_5: " << test3_4 << std::endl;

    std::cout << "     test4: " << test4 << std::endl;

    gp_float test5 =  func();
    
    std::cout << "     test5: " << test5 << std::endl;

    gp_float test6 =  1.7 + (.5 - 1.7) / (std::pow(2., .8) - 1) *
              (std::pow((gp_float)3, .8) - std::pow((gp_float)3 - 1., .8));
    
    std::cout << "     test6: " << test6 << std::endl;
}