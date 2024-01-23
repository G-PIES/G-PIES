#include "gtest/gtest.h"
#include "cluster_dynamics.hpp"
#include "client_db.hpp"

class ClientDbTest : public ::testing::Test
{
  protected:
    NuclearReactor reactor;
    Material material;
    ClientDb db;

    ClientDbTest() : db(nullptr) {}

    virtual ~ClientDbTest() {}

    virtual void SetUp()
    {
      nuclear_reactors::OSIRIS(reactor);
      materials::SA304(material);

      db = ClientDb(DEFAULT_CLIENT_DB_PATH, false);
      ASSERT_EQ(db.init(), SQLITE_OK);
    }

    virtual void TearDown()
    {
      ASSERT_EQ(db.clear(), SQLITE_OK);
    }
};

TEST_F(ClientDbTest, NuclearReactorCRUD)
{
  ASSERT_EQ(SQLITE_OK, db.create_reactor(reactor));
  ASSERT_EQ(0, reactor.sqlite_id);
  ASSERT_NE(nullptr, reactor.creation_datetime);
  ASSERT_NE(0, reactor.creation_datetime.length());

  NuclearReactor reactor_cpy;
  ASSERT_EQ(SQLITE_OK, db.read_reactor(reactor.sqlite_id, reactor_cpy));

  ASSERT_EQ(reactor.sqlite_id, reactor_cpy.sqlite_id);
  ASSERT_EQ(reactor.creation_datetime, reactor_cpy.creation_datetime);
  ASSERT_EQ(reactor.species, reactor_cpy.species);
  ASSERT_EQ(reactor.flux, reactor_cpy.flux);
  ASSERT_EQ(reactor.temperature, reactor_cpy.temperature);
  ASSERT_EQ(reactor.recombination, reactor_cpy.recombination);
  ASSERT_EQ(reactor.i_bi, reactor_cpy.i_bi);
  ASSERT_EQ(reactor.i_tri, reactor_cpy.i_tri);
  ASSERT_EQ(reactor.i_quad, reactor_cpy.i_quad);
  ASSERT_EQ(reactor.v_bi, reactor_cpy.v_bi);
  ASSERT_EQ(reactor.v_tri, reactor_cpy.v_tri);
  ASSERT_EQ(reactor.v_quad, reactor_cpy.v_quad);
  ASSERT_EQ(reactor.dislocation_density_evolution, reactor_cpy.dislocation_density_evolution);
}