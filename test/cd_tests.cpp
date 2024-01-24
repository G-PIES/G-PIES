#include "gtest/gtest.h"
#include "cluster_dynamics.hpp"

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