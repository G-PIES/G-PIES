#include "gtest/gtest.h"
#include "client_db.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

class ClientDbTest : public ::testing::Test
{
  protected:
    NuclearReactor reactor;
    Material material;
    ClientDb db;
    int sqlite_code = 0;

    ClientDbTest() : db(DEFAULT_CLIENT_DB_PATH, false) {}

    virtual ~ClientDbTest() {}

    virtual void SetUp()
    {
      nuclear_reactors::OSIRIS(reactor);
      materials::SA304(material);

      ASSERT_TRUE(db.clear(&sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

      ASSERT_TRUE(db.init(&sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    }

    virtual void TearDown()
    {
      ASSERT_TRUE(db.clear(&sqlite_code));
      ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
    }
};

TEST_F(ClientDbTest, NuclearReactorCRUD)
{
  ASSERT_TRUE(db.create_reactor(reactor, &sqlite_code));
  ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
  ASSERT_EQ(1, reactor.sqlite_id);
  ASSERT_NE("", reactor.creation_datetime);
  ASSERT_FALSE(reactor.creation_datetime.empty());

  NuclearReactor reactor_cpy;
  ASSERT_TRUE(db.read_reactor(reactor.sqlite_id, reactor_cpy, &sqlite_code));
  ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

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

  reactor_cpy.sqlite_id = -1;
  ASSERT_FALSE(db.read_reactor(10, reactor_cpy, &sqlite_code));
  ASSERT_THROW(db.read_reactor(-1, reactor_cpy, &sqlite_code), ClientDbException);

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