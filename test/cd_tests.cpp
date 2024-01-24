#include "gtest/gtest.h"
#include "cluster_dynamics.hpp"
#include "cluster_dynamics_impl.hpp"

class ClusterDynamicsTest : public ::testing::Test
{
  protected:
    NuclearReactor reactor;
    Material material;

    ClusterDynamicsTest() {}

    virtual ~ClusterDynamicsTest() {}

    virtual void SetUp()
    {
      nuclear_reactors::OSIRIS(reactor);
      materials::SA304(material);
    }

    virtual void TearDown() {}
};

TEST_F(ClusterDynamicsTest, ResultIsValid) 
{
  ClusterDynamics cd(100, reactor, material);
  ClusterDynamicsState state = cd.run(1e-5, 1e-5);
  ASSERT_EQ(state.valid, true);
}

TEST_F(ClusterDynamicsTest, CorrectEndtime) 
{
  ClusterDynamics cd(100, reactor, material);

  double delta_time = 1e-5;
  double total_time = 1.5e-3;

  ClusterDynamicsState state = cd.run(delta_time, total_time);

  // The real runtime may be greater than the provided one, but not by more than one delta_time
  ASSERT_GE(state.time, total_time);
  EXPECT_NEAR(state.time, total_time, delta_time);
}

TEST_F(ClusterDynamicsTest, ResultUnchanged)
{
  // Compare current results to old results to catch unexpected changes to results
  // Data generated with delta_time = 1e-5, total_time=1e-3, concentration_boundary=10

  const double interstitials_data[10] = {
    0, 
    1.99812e-10, 3.58921e-13, 1.61354e-11, 
    5.21253e-12, 5.28622e-38, 3.1901e-64, 
    1.36573e-90, 4.51776e-117, 1.2146e-143
  };
  const double vacancies_data[10] = {
    0,
    7.743e-11, 5.22e-12, 2.61e-12,
    1.74e-12, 7.23e-44, 1.82162e-75,
    3.30383e-107, 4.68642e-139, 5.45938e-171
  };
  const double dislocation_density_data = 1e-13;

  ClusterDynamics cd(10, reactor, material);
  ClusterDynamicsState state = cd.run(1e-5, 1e-3);
  for (size_t i = 1; i < 10; ++i)
  {
    EXPECT_NEAR(state.interstitials[i], interstitials_data[i], 1e-7);
    EXPECT_NEAR(state.vacancies[i], vacancies_data[i], 1e-7);
  }

  EXPECT_NEAR(state.dislocation_density, dislocation_density_data, 1e-7);
}

TEST_F(ClusterDynamicsTest, i_defect_production_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[5] = {
    2.0879999999999996e-08, 
    4.3499999999999992e-08, 
    1.7399999999999997e-08, 
    5.2199999999999989e-09, 
    0.0
  };
  double actual[5];
  for (int i = 0; i < 5; ++i)
  {
    actual[i] = cd.i_defect_production(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, v_defect_production_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[5] = {
    7.7429999999999973e-08, 
    5.2199999999999989e-09, 
    2.6099999999999995e-09, 
    1.7399999999999998e-09, 
    0.0
  };
  double actual[5];
  for (int i = 0; i < 5; ++i)
  {
    actual[i] = cd.v_defect_production(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
  EXPECT_DOUBLE_EQ(expected[3], actual[3]);
  EXPECT_DOUBLE_EQ(expected[4], actual[4]);
}

TEST_F(ClusterDynamicsTest, annihilation_rate_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  double expected_annihilation_rate = 1.5282982144235406e-13;
  double actual_annihilation_rate = cd.annihilation_rate();

  EXPECT_DOUBLE_EQ(expected_annihilation_rate, actual_annihilation_rate);
}

TEST_F(ClusterDynamicsTest, ii_emission_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[3] = {
    2110271810466722.8,
    128124.97669171244,
    155.23748555687555
  };
  double actual[3];
  for (int i = 0; i < 3; ++i) 
  {
    actual[i] = cd.ii_emission(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, vv_emission_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[3] = {
    3.0908391442760997e-19,
    1.0182440692940207e-22,
    1.0522976212093976e-23
  };
  double actual[3];
  for (int i = 0; i < 3; ++i) 
  {
    actual[i] = cd.vv_emission(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, ii_absorption_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[3] = {
    1.5417473401310976e-13,
    1.5415520574903579e-13,
    1.5510246725819022e-13
  };
  double actual[3];
  for (int i = 0; i < 3; ++i) 
  {
    actual[i] = cd.ii_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, vi_absorption_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[3] = {
    1.5417473401310976e-13,
    1.5415520574903579e-13,
    1.5510246725819022e-13
  };
  double actual[3];
  for (int i = 0; i < 3; ++i) 
  {
    actual[i] = cd.vi_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, iv_absorption_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[3] = {
    1.4625897359941745e-18,
    1.5337313446993508e-18,
    1.5883202210270714e-18
  };
  double actual[3];
  for (int i = 0; i < 3; ++i) 
  {
    actual[i] = cd.iv_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, vv_absorption_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  // Starts from cluster size = 1
  double expected[3] = {
    1.4625897359941745e-18,
    1.5337313446993508e-18,
    1.5883202210270714e-18
  };
  double actual[3];
  for (int i = 0; i < 3; ++i) 
  {
    actual[i] = cd.vv_absorption(i + 1);
  }

  EXPECT_DOUBLE_EQ(expected[0], actual[0]);
  EXPECT_DOUBLE_EQ(expected[1], actual[1]);
  EXPECT_DOUBLE_EQ(expected[2], actual[2]);
}

TEST_F(ClusterDynamicsTest, i_dislocation_annihilation_time_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  double expected = 1.3898960585069451e-20;
  double actual = cd.i_dislocation_annihilation_time();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_dislocation_annihilation_time_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  double expected = 2.0452274646486708e-25;
  double actual = cd.v_dislocation_annihilation_time();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, i_grain_boundary_annihilation_time_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  double expected = 7.3710369608603651e-11;
  double actual = cd.i_grain_boundary_annihilation_time();

  EXPECT_DOUBLE_EQ(expected, actual);
}

TEST_F(ClusterDynamicsTest, v_grain_boundary_annihilation_time_test)
{
  ClusterDynamicsImpl cd(10, reactor, material);

  double expected = 1.2033063175360925e-15;
  double actual = cd.v_grain_boundary_annihilation_time();

  EXPECT_DOUBLE_EQ(expected, actual);
}

// TEST_F(ClusterDynamicsTest, i_emission_time_test)
// {
//   ClusterDynamicsImpl cd(10, reactor, material);

//   double expected = 1.2033063175360925e-15;
//   double actual = cd.i_emission_time(3);

//   EXPECT_DOUBLE_EQ(expected, actual);
// }

// TEST_F(ClusterDynamicsTest, v_emission_time_test)
// {
//   ClusterDynamicsImpl cd(10, reactor, material);

//   double expected = 1.2033063175360925e-15;
//   double actual = cd.v_emission_time(3);

//   EXPECT_DOUBLE_EQ(expected, actual);
// }