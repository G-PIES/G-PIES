#include "cluster_dynamics/cluster_dynamics.hpp"
#include "gtest/gtest.h"
#include "gtest_helpers.hpp"

#if defined(USE_CUDA)
#include "cluster_dynamics_cuda_impl.hpp"
#elif defined(USE_METAL)
#include "cluster_dynamics_metal_impl.hpp"
#else
#include "cluster_dynamics_impl.hpp"
#endif

#include <numeric>

class ClusterDynamicsTest : public ::testing::Test {
 protected:
  NuclearReactor reactor;
  Material material;

  static constexpr size_t SIM_SIZE = 10;
  static constexpr double SIM_time_delta = 1e-5;
  static constexpr double SIM_RUN_TIME = 1e-5;

  ClusterDynamicsTest() {}

  virtual ~ClusterDynamicsTest() {}

  virtual void SetUp() {
    nuclear_reactors::OSIRIS(reactor);
    materials::SA304(material);
  }

  virtual void TearDown() {}
};

TEST_F(ClusterDynamicsTest, ResultIsValid) {
  ClusterDynamics cd(100, reactor, material);
  EXPECT_NO_THROW(ClusterDynamicsState state =
                      cd.run(SIM_time_delta, SIM_RUN_TIME););
}

TEST_F(ClusterDynamicsTest, CorrectEndtime) {
  ClusterDynamics cd(100, reactor, material);

  gp_float time_delta = 1e-5;
  gp_float total_time = 1.5e-3;

  ClusterDynamicsState state = cd.run(time_delta, total_time);

  // The real runtime may be greater than the provided one, but not by more
  // than one time_delta
  ASSERT_GE(state.time, total_time);
  EXPECT_NEAR(state.time, total_time, time_delta);
}

TEST_F(ClusterDynamicsTest, ResultUnchanged) {
  // Compare current results to old results to catch unexpected changes to
  // results Data generated with time_delta = 1e-5, total_time=1e-3,
  // max_cluster_size=10

  const gp_float interstitials_data[10] = {
      0,           1.99812e-10, 3.58921e-13, 1.61354e-11,  5.21253e-12,
      5.28622e-38, 3.1901e-64,  1.36573e-90, 4.51776e-117, 1.2146e-143};
  const gp_float vacancies_data[10] = {
      0,        7.743e-11,   5.22e-12,     2.61e-12,     1.74e-12,
      7.23e-44, 1.82162e-75, 3.30383e-107, 4.68642e-139, 5.45938e-171};
  const gp_float dislocation_density_data = 1e-13;

  ClusterDynamics cd(10, reactor, material);
  ClusterDynamicsState state = cd.run(1e-5, 1e-3);
  for (size_t i = 1; i < 10; ++i) {
    EXPECT_NEAR(state.interstitials[i], interstitials_data[i], 1e-7);
    EXPECT_NEAR(state.vacancies[i], vacancies_data[i], 1e-7);
  }

  EXPECT_NEAR(state.dislocation_density, dislocation_density_data, 1e-7);
}

TEST_F(ClusterDynamicsTest, i_concentration_derivative_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {2.7011322486896344e-11, 1.7373139700233559e-08,
                          5.2198489772795402e-09, 5.2527951507159676e-39, 0.0};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_concentration_derivative(i + 2);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
  GP_EXPECT_NEAR(expected[3], actual[3]);
  GP_EXPECT_NEAR(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_concentration_derivative_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {5.2199999999999989e-09, 2.6099999999999995e-09,
                          1.7399999999999998e-09, 2.2019179047621176e-44, 0.0};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_concentration_derivative(i + 2);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, i1_concentration_derivative_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 6.4461184990181082e-08;
  gp_float actual = cd.i1_concentration_derivative();

  GP_EXPECT_NEAR(expected, actual);
}

TEST_F(ClusterDynamicsTest, v1_concentration_derivative_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 7.7429999999999973e-08;
  gp_float actual = cd.v1_concentration_derivative();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, i_demotion_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {155.23748555687555, 2.8931555643430293,
                          0.17853595808953046, 0.021470762169837978,
                          0.0039393178609528059};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_demotion_rate(i + 3);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
  GP_EXPECT_NEAR(expected[3], actual[3]);
  GP_EXPECT_NEAR(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_demotion_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {1.0622831180514798e-23, 2.857269498818864e-24,
                          1.1855601898843091e-24, 6.366880053300996e-25,
                          4.0695742442086016e-25};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_demotion_rate(i + 3);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
  GP_EXPECT_NEAR(expected[3], actual[3]);
  GP_EXPECT_NEAR(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, i_combined_promotion_demotion_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {128124.97669171244, 155.23748555687555,
                          2.8931555643430293, 0.17853595808953046,
                          0.021470762169837978};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_combined_promotion_demotion_rate(i + 2);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
  GP_EXPECT_NEAR(expected[3], actual[3]);
  GP_EXPECT_NEAR(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_combined_promotion_demotion_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {1.0192365323843148e-22, 1.0622832410351147e-23,
                          2.8572707642889243e-24, 1.1855614867483006e-24,
                          6.3668933057637471e-25};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_combined_promotion_demotion_rate(i + 2);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
  GP_EXPECT_NEAR(expected[3], actual[3]);
  GP_EXPECT_NEAR(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, i_promotion_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {9.9257693757640059e-26, 9.924512146122923e-26,
                          9.9854968420822849e-26, 1.0062825959225992e-25,
                          1.0145241391493604e-25};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_promotion_rate(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_promotion_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 2
  gp_float expected[5] = {1.1324832325802889e-30, 1.1875681802007069e-30,
                          1.229836347141261e-30, 1.2654700602081136e-30,
                          1.2968639916403274e-30};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_promotion_rate(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, i_defect_production_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 1
  gp_float expected[5] = {6.4379999999999985e-08, 0.0, 1.7399999999999997e-08,
                          5.2199999999999989e-09, 0.0};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_defect_production(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_defect_production_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 1
  gp_float expected[5] = {7.7429999999999973e-08, 5.2199999999999989e-09,
                          2.6099999999999995e-09, 1.7399999999999998e-09, 0.0};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_defect_production(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, annihilation_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.step_init();

  gp_float expected = 1.5282982144235406e-13;
  gp_float actual = cd.annihilation_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, ii_emission_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.step_init();

  // Starts from cluster size = 1
  gp_float expected[3] = {2110271810466722.8, 128124.97669171244,
                          155.23748555687555};
  gp_float actual[3];
  for (int i = 0; i < 3; ++i) {
    actual[i] = cd.ii_emission(i + 1);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, vv_emission_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.step_init();

  // Starts from cluster size = 1
  gp_float expected[3] = {3.0908391442760997e-19, 1.0182440692940207e-22,
                          1.0522976212093976e-23};
  gp_float actual[3];
  for (int i = 0; i < 3; ++i) {
    actual[i] = cd.vv_emission(i + 1);
  }

  GP_EXPECT_NEAR(expected[0], actual[0]);
  GP_EXPECT_NEAR(expected[1], actual[1]);
  GP_EXPECT_NEAR(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, ii_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 1
  gp_float expected[3] = {1.5417473401310976e-13, 1.5415520574903579e-13,
                          1.5510246725819022e-13};
  gp_float actual[3];
  for (int i = 0; i < 3; ++i) {
    actual[i] = cd.ii_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, vi_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 1
  gp_float expected[3] = {1.5417473401310976e-13, 1.5415520574903579e-13,
                          1.5510246725819022e-13};
  gp_float actual[3];
  for (int i = 0; i < 3; ++i) {
    actual[i] = cd.vi_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, iv_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 1
  gp_float expected[3] = {1.4625897359941745e-18, 1.5337313446993508e-18,
                          1.5883202210270714e-18};
  gp_float actual[3];
  for (int i = 0; i < 3; ++i) {
    actual[i] = cd.iv_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, vv_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  // Starts from cluster size = 1
  gp_float expected[3] = {1.4625897359941745e-18, 1.5337313446993508e-18,
                          1.5883202210270714e-18};
  gp_float actual[3];
  for (int i = 0; i < 3; ++i) {
    actual[i] = cd.vv_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, i_dislocation_annihilation_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.3898960585069451e-20;
  gp_float actual = cd.i_dislocation_annihilation_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_dislocation_annihilation_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 2.0452274646486708e-25;
  gp_float actual = cd.v_dislocation_annihilation_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, i_grain_boundary_annihilation_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 7.3710369608603651e-11;
  gp_float actual = cd.i_grain_boundary_annihilation_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_grain_boundary_annihilation_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.2033063175360925e-15;
  gp_float actual = cd.v_grain_boundary_annihilation_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, i_emission_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 8.1184990181147742e-11;
  gp_float actual = cd.i_emission_rate();

  GP_EXPECT_NEAR(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_emission_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 2.1588731998897621e-35;
  gp_float actual = cd.v_emission_rate();

  GP_EXPECT_NEAR(expected, actual);
}

TEST_F(ClusterDynamicsTest, i_absorption_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 4.9961636205194337e-26;
  gp_float actual = cd.i_absorption_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_absorption_rate_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.6441170027726276e-30;
  gp_float actual = cd.v_absorption_rate();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, i_bias_factor_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 1
  gp_float expected[5] = {10.567277704705269, 7.4712472717635334,
                          6.1377329960200999, 5.3565961679959591,
                          4.8303246748035633};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_bias_factor(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_bias_factor_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 1
  gp_float expected[5] = {5.535240702464665, 4.104386828441297,
                          3.4704950346240908, 3.0926203512323323,
                          2.8347460594319629};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_bias_factor(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, i_binding_energy_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 2
  gp_float expected[5] = {0.59999999999999964, 0.94937403057784353,
                          1.1567720263492856, 1.3019633024687201,
                          1.4124782726995702};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.i_binding_energy(i + 2);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_binding_energy_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 2
  gp_float expected[5] = {0.5, 0.61978538191240373, 0.69089326617689806,
                          0.74067313227498988, 0.7785639792112814};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.v_binding_energy(i + 2);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, i_diffusion_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  gp_float expected = 1.7373700731336814e-07;
  gp_float actual = cd.i_diffusion();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_diffusion_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  gp_float expected = 3.1465037917671854e-12;
  gp_float actual = cd.v_diffusion();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, dislocation_promotion_probability_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  cd.step_init();

  // Starts from cluster size = 2
  gp_float expected[5] = {2.2743949558520322e-29, 2.2743949558520299e-29,
                          2.2743949558520339e-29, 2.2743949558520277e-29,
                          2.2743949558520367e-29};

  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.dislocation_promotion_probability(i + 2);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, cluster_radius_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());

  // Starts from cluster size = 2
  gp_float expected[5] = {1.890135244455595e-08, 2.3149334468834776e-08,
                          2.6730548974284879e-08, 2.9885662291194137e-08,
                          3.2738102765737105e-08};
  gp_float actual[5];
  for (int i = 0; i < 5; ++i) {
    actual[i] = cd.cluster_radius(i + 2);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, update_clusters_1_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.step_init();
  cd.update_clusters_1(1e-5);

  // Starts from cluster size = 2
  gp_float i_expected = 0.0;
  gp_float v_expected = 0.0;
  gp_float i_actual = cd.interstitials[1];
  gp_float v_actual = cd.vacancies[1];

  EXPECT_DOUBLE_EQ(i_expected, i_actual);
  EXPECT_DOUBLE_EQ(v_expected, v_actual);
}

TEST_F(ClusterDynamicsTest, update_clusters_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.update_clusters(1e-5);

  // Starts from cluster size = 2
  gp_float i_expected[5] = {0.0, 1.7399999999999999e-13, 5.2199999999999995e-14,
                            0.0, 0.0};
  gp_float v_expected[5] = {5.2199999999999995e-14, 2.6099999999999997e-14,
                            1.74e-14, 0.0, 0.0};
  gp_float i_actual[5];
  gp_float v_actual[5];
  for (int i = 0; i < 5; ++i) {
    i_actual[i] = cd.interstitials_temp[i + 2];
    v_actual[i] = cd.vacancies_temp[i + 2];
  }

  EXPECT_DOUBLE_EQ(i_expected[0], i_actual[0]);
  EXPECT_DOUBLE_EQ(i_expected[1], i_actual[1]);
  EXPECT_DOUBLE_EQ(i_expected[2], i_actual[2]);
  EXPECT_DOUBLE_EQ(i_expected[3], i_actual[3]);
  EXPECT_DOUBLE_EQ(i_expected[4], i_actual[4]);

  EXPECT_DOUBLE_EQ(v_expected[0], v_actual[0]);
  EXPECT_DOUBLE_EQ(v_expected[1], v_actual[1]);
  EXPECT_DOUBLE_EQ(v_expected[2], v_actual[2]);
  EXPECT_DOUBLE_EQ(v_expected[3], v_actual[3]);
  EXPECT_DOUBLE_EQ(v_expected[4], v_actual[4]);
}

TEST_F(ClusterDynamicsTest, mean_dislocation_cell_radius_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 0.0064902708966673205;
  gp_float actual = cd.mean_dislocation_cell_radius();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, ii_sum_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.3440457113561326e-25;
  gp_float actual = cd.ii_sum_absorption(cd.max_cluster_size - 1);

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, vi_sum_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.3419225537357197e-25;
  gp_float actual = cd.vi_sum_absorption(cd.max_cluster_size - 1);

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, iv_sum_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.3032955788203968e-30;
  gp_float actual = cd.iv_sum_absorption(cd.max_cluster_size - 1);

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, vv_sum_absorption_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.run(SIM_time_delta, SIM_RUN_TIME);

  gp_float expected = 1.2824366959852804e-30;
  gp_float actual = cd.vv_sum_absorption(cd.max_cluster_size - 1);

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, step_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.step(1e-5);

  // Starts from cluster size = 1
  gp_float i_expected[5] = {6.4379999999999994e-13, 0.0, 1.7399999999999999e-13,
                            5.2199999999999995e-14, 0.0};
  gp_float v_expected[5] = {
      7.7429999999999974e-13,
      5.2199999999999995e-14,
      2.6099999999999997e-14,
      1.74e-14,
      0.0,
  };
  gp_float i_actual[5];
  gp_float v_actual[5];
  for (int i = 0; i < 5; ++i) {
    i_actual[i] = cd.interstitials_temp[i + 1];
    v_actual[i] = cd.vacancies_temp[i + 1];
  }

  EXPECT_DOUBLE_EQ(i_expected[0], i_actual[0]);
  EXPECT_DOUBLE_EQ(i_expected[1], i_actual[1]);
  EXPECT_DOUBLE_EQ(i_expected[2], i_actual[2]);
  EXPECT_DOUBLE_EQ(i_expected[3], i_actual[3]);
  EXPECT_DOUBLE_EQ(i_expected[4], i_actual[4]);

  EXPECT_DOUBLE_EQ(v_expected[0], v_actual[0]);
  EXPECT_DOUBLE_EQ(v_expected[1], v_actual[1]);
  EXPECT_DOUBLE_EQ(v_expected[2], v_actual[2]);
  EXPECT_DOUBLE_EQ(v_expected[3], v_actual[3]);
  EXPECT_DOUBLE_EQ(v_expected[4], v_actual[4]);
}

TEST_F(ClusterDynamicsTest, validation_test) {
  ClusterDynamicsImpl cd(SIM_SIZE, *reactor.impl(), *material.impl());
  cd.interstitials_temp[1] = NAN;
  cd.interstitials_temp[3] = INFINITY;
  cd.interstitials_temp[4] = -1.0;

  EXPECT_THROW(cd.validate(1), ClusterDynamicsException);
  EXPECT_THROW(cd.validate(3), ClusterDynamicsException);
  EXPECT_THROW(cd.validate(4), ClusterDynamicsException);
}
