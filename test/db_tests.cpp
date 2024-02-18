#include "gtest/gtest.h"
#include "client_db/client_db.hpp"
#include "model/nuclear_reactor.hpp"
#include "model/material.hpp"
#include "utils/randomizer.hpp"

class ClientDbTest : public ::testing::Test
{
  protected:
    Randomizer randomizer;
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
   // create one 
   ASSERT_TRUE(db.create_reactor(reactor, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(1, reactor.sqlite_id);

   // read one
   NuclearReactor reactor_cpy;
   ASSERT_TRUE(db.read_reactor(reactor.sqlite_id, reactor_cpy, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

   // read one equality
   ASSERT_EQ(reactor.sqlite_id, reactor_cpy.sqlite_id);
   ASSERT_EQ(reactor.creation_datetime, reactor_cpy.creation_datetime);
   ASSERT_EQ(reactor.species, reactor_cpy.species);
   ASSERT_EQ(reactor.get_flux(), reactor_cpy.get_flux());
   ASSERT_EQ(reactor.get_temperature(), reactor_cpy.get_temperature());
   ASSERT_EQ(reactor.get_recombination(), reactor_cpy.get_recombination());
   ASSERT_EQ(reactor.get_i_bi(), reactor_cpy.get_i_bi());
   ASSERT_EQ(reactor.get_i_tri(), reactor_cpy.get_i_tri());
   ASSERT_EQ(reactor.get_i_quad(), reactor_cpy.get_i_quad());
   ASSERT_EQ(reactor.get_v_bi(), reactor_cpy.get_v_bi());
   ASSERT_EQ(reactor.get_v_tri(), reactor_cpy.get_v_tri());
   ASSERT_EQ(reactor.get_v_quad(), reactor_cpy.get_v_quad());
   ASSERT_EQ(reactor.get_dislocation_density_evolution(), reactor_cpy.get_dislocation_density_evolution());

   // read one id not found, sql success
   reactor_cpy.sqlite_id = -1;
   ASSERT_FALSE(db.read_reactor(10, reactor_cpy, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

   // read one invalid sql id
   ASSERT_THROW(db.read_reactor(-1, reactor_cpy, &sqlite_code), ClientDbException);

   // object equality after failed reads
   ASSERT_EQ(reactor.creation_datetime, reactor_cpy.creation_datetime);
   ASSERT_EQ(reactor.species, reactor_cpy.species);
   ASSERT_EQ(reactor.get_flux(), reactor_cpy.get_flux());
   ASSERT_EQ(reactor.get_temperature(), reactor_cpy.get_temperature());
   ASSERT_EQ(reactor.get_recombination(), reactor_cpy.get_recombination());
   ASSERT_EQ(reactor.get_i_bi(), reactor_cpy.get_i_bi());
   ASSERT_EQ(reactor.get_i_tri(), reactor_cpy.get_i_tri());
   ASSERT_EQ(reactor.get_i_quad(), reactor_cpy.get_i_quad());
   ASSERT_EQ(reactor.get_v_bi(), reactor_cpy.get_v_bi());
   ASSERT_EQ(reactor.get_v_tri(), reactor_cpy.get_v_tri());
   ASSERT_EQ(reactor.get_v_quad(), reactor_cpy.get_v_quad());
   ASSERT_EQ(reactor.get_dislocation_density_evolution(), reactor_cpy.get_dislocation_density_evolution());

   // update one
   randomizer.reactor_randomize(reactor);
   ASSERT_TRUE(db.update_reactor(reactor, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(1, db.changes());

   // read one after update
   ASSERT_TRUE(db.read_reactor(reactor.sqlite_id, reactor_cpy, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

   // read one equality after update
   ASSERT_EQ(reactor.sqlite_id, reactor_cpy.sqlite_id);
   ASSERT_EQ(reactor.creation_datetime, reactor_cpy.creation_datetime);
   ASSERT_EQ(reactor.species, reactor_cpy.species);
   ASSERT_EQ(reactor.get_flux(), reactor_cpy.get_flux());
   ASSERT_EQ(reactor.get_temperature(), reactor_cpy.get_temperature());
   ASSERT_EQ(reactor.get_recombination(), reactor_cpy.get_recombination());
   ASSERT_EQ(reactor.get_i_bi(), reactor_cpy.get_i_bi());
   ASSERT_EQ(reactor.get_i_tri(), reactor_cpy.get_i_tri());
   ASSERT_EQ(reactor.get_i_quad(), reactor_cpy.get_i_quad());
   ASSERT_EQ(reactor.get_v_bi(), reactor_cpy.get_v_bi());
   ASSERT_EQ(reactor.get_v_tri(), reactor_cpy.get_v_tri());
   ASSERT_EQ(reactor.get_v_quad(), reactor_cpy.get_v_quad());
   ASSERT_EQ(reactor.get_dislocation_density_evolution(), reactor_cpy.get_dislocation_density_evolution());

   // update one invalid sql id
   reactor_cpy.sqlite_id = -1;
   ASSERT_THROW(db.update_reactor(reactor_cpy, &sqlite_code), ClientDbException);

   // delete one
   ASSERT_TRUE(db.delete_reactor(reactor, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(1, db.changes());
   
   // delete one not found
   ASSERT_TRUE(db.delete_reactor(reactor, &sqlite_code));
   ASSERT_EQ(0, db.changes());

   // delete one invalid sql id
   reactor_cpy.sqlite_id = -1;
   ASSERT_THROW(db.delete_reactor(reactor_cpy, &sqlite_code), ClientDbException);
}

TEST_F(ClientDbTest, NuclearReactorManyCRUD)
{
   std::vector<NuclearReactor> reactors;

   // create 20 reactors
   for (int i = 0; i < 20; ++i)
   {
       NuclearReactor reactor;
       randomizer.reactor_randomize(reactor);
       ASSERT_TRUE(db.create_reactor(reactor, &sqlite_code));
       ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
       ASSERT_EQ(1 + i, reactor.sqlite_id);
       reactors.push_back(reactor);
   }

   // read 20 reactors
   std::vector<NuclearReactor> reactor_reads;
   ASSERT_TRUE(db.read_reactors(reactor_reads, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(20u, reactor_reads.size());
   ASSERT_EQ(reactors.size(), reactor_reads.size());

   // update 20 reactors
   for (int i = 0; i < 20; ++i)
   {
       randomizer.reactor_randomize(reactors[i]);
       ASSERT_TRUE(db.update_reactor(reactors[i], &sqlite_code));
       ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   }

   // delete 20 reactors
   for (int i = 0; i < 20; ++i)
   {
       ASSERT_TRUE(db.delete_reactor(reactors[i], &sqlite_code));
       ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   }
}

TEST_F(ClientDbTest, MaterialCRUD)
{
   // create one 
   ASSERT_TRUE(db.create_material(material, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(1, material.sqlite_id);

   // read one
   Material material_cpy;
   ASSERT_TRUE(db.read_material(material.sqlite_id, material_cpy, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

   // read one equality
   ASSERT_EQ(material.sqlite_id, material_cpy.sqlite_id);
   ASSERT_EQ(material.creation_datetime, material_cpy.creation_datetime);
   ASSERT_EQ(material.species, material_cpy.species);
   ASSERT_EQ(material.get_i_migration(), material_cpy.get_i_migration());
   ASSERT_EQ(material.get_v_migration(), material_cpy.get_v_migration());
   ASSERT_EQ(material.get_i_diffusion_0(), material_cpy.get_i_diffusion_0());
   ASSERT_EQ(material.get_v_diffusion_0(), material_cpy.get_v_diffusion_0());
   ASSERT_EQ(material.get_i_formation(), material_cpy.get_i_formation());
   ASSERT_EQ(material.get_v_formation(), material_cpy.get_v_formation());
   ASSERT_EQ(material.get_i_binding(), material_cpy.get_i_binding());
   ASSERT_EQ(material.get_v_binding(), material_cpy.get_v_binding());
   ASSERT_EQ(material.get_recombination_radius(), material_cpy.get_recombination_radius());
   ASSERT_EQ(material.get_i_loop_bias(), material_cpy.get_i_loop_bias());
   ASSERT_EQ(material.get_i_dislocation_bias(), material_cpy.get_i_dislocation_bias());
   ASSERT_EQ(material.get_i_dislocation_bias_param(), material_cpy.get_i_dislocation_bias_param());
   ASSERT_EQ(material.get_v_loop_bias(), material_cpy.get_v_loop_bias());
   ASSERT_EQ(material.get_v_dislocation_bias(), material_cpy.get_v_dislocation_bias());
   ASSERT_EQ(material.get_v_dislocation_bias_param(), material_cpy.get_v_dislocation_bias_param());
   ASSERT_EQ(material.get_dislocation_density_0(), material_cpy.get_dislocation_density_0());
   ASSERT_EQ(material.get_grain_size(), material_cpy.get_grain_size());
   ASSERT_EQ(material.get_lattice_param(), material_cpy.get_lattice_param());
   ASSERT_EQ(material.get_burgers_vector(), material_cpy.get_burgers_vector());
   ASSERT_EQ(material.get_atomic_volume(), material_cpy.get_atomic_volume());

   // read one id not found, sql success
   material_cpy.sqlite_id = -1;
   ASSERT_FALSE(db.read_material(10, material_cpy, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

   // read one invalid sql id
   ASSERT_THROW(db.read_material(-1, material_cpy, &sqlite_code), ClientDbException);

   // object equality after failed reads
   ASSERT_EQ(material.creation_datetime, material_cpy.creation_datetime);
   ASSERT_EQ(material.species, material_cpy.species);
   ASSERT_EQ(material.get_i_migration(), material_cpy.get_i_migration());
   ASSERT_EQ(material.get_v_migration(), material_cpy.get_v_migration());
   ASSERT_EQ(material.get_i_diffusion_0(), material_cpy.get_i_diffusion_0());
   ASSERT_EQ(material.get_v_diffusion_0(), material_cpy.get_v_diffusion_0());
   ASSERT_EQ(material.get_i_formation(), material_cpy.get_i_formation());
   ASSERT_EQ(material.get_v_formation(), material_cpy.get_v_formation());
   ASSERT_EQ(material.get_i_binding(), material_cpy.get_i_binding());
   ASSERT_EQ(material.get_v_binding(), material_cpy.get_v_binding());
   ASSERT_EQ(material.get_recombination_radius(), material_cpy.get_recombination_radius());
   ASSERT_EQ(material.get_i_loop_bias(), material_cpy.get_i_loop_bias());
   ASSERT_EQ(material.get_i_dislocation_bias(), material_cpy.get_i_dislocation_bias());
   ASSERT_EQ(material.get_i_dislocation_bias_param(), material_cpy.get_i_dislocation_bias_param());
   ASSERT_EQ(material.get_v_loop_bias(), material_cpy.get_v_loop_bias());
   ASSERT_EQ(material.get_v_dislocation_bias(), material_cpy.get_v_dislocation_bias());
   ASSERT_EQ(material.get_v_dislocation_bias_param(), material_cpy.get_v_dislocation_bias_param());
   ASSERT_EQ(material.get_dislocation_density_0(), material_cpy.get_dislocation_density_0());
   ASSERT_EQ(material.get_grain_size(), material_cpy.get_grain_size());
   ASSERT_EQ(material.get_lattice_param(), material_cpy.get_lattice_param());
   ASSERT_EQ(material.get_burgers_vector(), material_cpy.get_burgers_vector());
   ASSERT_EQ(material.get_atomic_volume(), material_cpy.get_atomic_volume());

   // update one
   randomizer.material_randomize(material);
   ASSERT_TRUE(db.update_material(material, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(1, db.changes());

   // read one after update
   ASSERT_TRUE(db.read_material(material.sqlite_id, material_cpy, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));

   // read one equality after update
   ASSERT_EQ(material.sqlite_id, material_cpy.sqlite_id);
   ASSERT_EQ(material.creation_datetime, material_cpy.creation_datetime);
   ASSERT_EQ(material.species, material_cpy.species);
   ASSERT_EQ(material.get_i_migration(), material_cpy.get_i_migration());
   ASSERT_EQ(material.get_v_migration(), material_cpy.get_v_migration());
   ASSERT_EQ(material.get_i_diffusion_0(), material_cpy.get_i_diffusion_0());
   ASSERT_EQ(material.get_v_diffusion_0(), material_cpy.get_v_diffusion_0());
   ASSERT_EQ(material.get_i_formation(), material_cpy.get_i_formation());
   ASSERT_EQ(material.get_v_formation(), material_cpy.get_v_formation());
   ASSERT_EQ(material.get_i_binding(), material_cpy.get_i_binding());
   ASSERT_EQ(material.get_v_binding(), material_cpy.get_v_binding());
   ASSERT_EQ(material.get_recombination_radius(), material_cpy.get_recombination_radius());
   ASSERT_EQ(material.get_i_loop_bias(), material_cpy.get_i_loop_bias());
   ASSERT_EQ(material.get_i_dislocation_bias(), material_cpy.get_i_dislocation_bias());
   ASSERT_EQ(material.get_i_dislocation_bias_param(), material_cpy.get_i_dislocation_bias_param());
   ASSERT_EQ(material.get_v_loop_bias(), material_cpy.get_v_loop_bias());
   ASSERT_EQ(material.get_v_dislocation_bias(), material_cpy.get_v_dislocation_bias());
   ASSERT_EQ(material.get_v_dislocation_bias_param(), material_cpy.get_v_dislocation_bias_param());
   ASSERT_EQ(material.get_dislocation_density_0(), material_cpy.get_dislocation_density_0());
   ASSERT_EQ(material.get_grain_size(), material_cpy.get_grain_size());
   ASSERT_EQ(material.get_lattice_param(), material_cpy.get_lattice_param());
   ASSERT_EQ(material.get_burgers_vector(), material_cpy.get_burgers_vector());
   ASSERT_EQ(material.get_atomic_volume(), material_cpy.get_atomic_volume());

   // update one invalid sql id
   material_cpy.sqlite_id = -1;
   ASSERT_THROW(db.update_material(material_cpy, &sqlite_code), ClientDbException);

   // delete one
   ASSERT_TRUE(db.delete_material(material, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(1, db.changes());
   
   // delete one not found
   ASSERT_TRUE(db.delete_material(material, &sqlite_code));
   ASSERT_EQ(0, db.changes());

   // delete one invalid sql id
   material_cpy.sqlite_id = -1;
   ASSERT_THROW(db.delete_material(material_cpy, &sqlite_code), ClientDbException);
}

TEST_F(ClientDbTest, MaterialManyCRUD)
{
   std::vector<Material> materials;

   // create 20 materials
   for (int i = 0; i < 20; ++i)
   {
       Material material;
       randomizer.material_randomize(material);
       ASSERT_TRUE(db.create_material(material, &sqlite_code));
       ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
       ASSERT_EQ(1 + i, material.sqlite_id);
       materials.push_back(material);
   }

   // read 20 materials
   std::vector<Material> material_reads;
   ASSERT_TRUE(db.read_materials(material_reads, &sqlite_code));
   ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   ASSERT_EQ(20u, material_reads.size());
   ASSERT_EQ(materials.size(), material_reads.size());

   // update 20 materials
   for (int i = 0; i < 20; ++i)
   {
       randomizer.material_randomize(materials[i]);
       ASSERT_TRUE(db.update_material(materials[i], &sqlite_code));
       ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   }

   // delete 20 materials
   for (int i = 0; i < 20; ++i)
   {
       ASSERT_TRUE(db.delete_material(materials[i], &sqlite_code));
       ASSERT_TRUE(db.is_sqlite_success(sqlite_code));
   }
}