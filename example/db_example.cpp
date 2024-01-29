#include <iostream>
#include <vector>

#include "client_db.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"
#include "gpies_exception.hpp"
#include "randomizer.hpp"

#define VEC_SIZE 4

double randd();
void reactors_crud();
void reactor_cmp_print(NuclearReactor&, NuclearReactor&);
void materials_crud();
void material_cmp_print(Material&, Material&);

ClientDb db;
Randomizer randomizer;

int main(int argc, char* argv[])
{
    try
    {
        db.clear();
        db.init();

        std::cout << "\nREACTORS CRUD -----------------------------------\n";
        reactors_crud();

        std::cout << "\nMATERIALS CRUD ----------------------------------\n";
        materials_crud();
    }
    catch(const ClientDbException& e)
    {
        fprintf(stderr, "%s\n%s sqlite code = %4d\n\n** SQL QUERY **\n%s\n\n",
            e.message.c_str(), e.sqlite_errmsg.c_str(), e.sqlite_code, e.query.c_str());
    }

    return 0;
}

double randd()
{
    return static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / 100));
}

void reactors_crud()
{
    int sqlite_code = -1;

    // generate a list of randomized reactors
    std::vector<NuclearReactor> reactors(VEC_SIZE, NuclearReactor());
    for (int i = 0; i < VEC_SIZE; ++i)
    {
        reactors[i].species = "G-PIES REACTOR " + std::to_string(i);
        randomizer.reactor_randomize(reactors[i]);

        db.create_reactor(reactors[i], &sqlite_code);

        fprintf(stdout, "* CREATE %s\t-\tid = %4d\t-\tsqlite code = %4d\n",
            reactors[i].species.c_str(), reactors[i].sqlite_id, sqlite_code);
    }

    fprintf(stdout, "\n");

    std::vector<NuclearReactor> read_reactors;
    db.read_reactors(read_reactors, &sqlite_code);
    fprintf(stdout, "* READ REACTORS - count = %4d - sqlite code = %4d\n" 
        "* EXISTING\t\t-\tREAD RESULT\n\n",
        (int)read_reactors.size(), sqlite_code);

    NuclearReactor* existing;
    NuclearReactor* read;

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        reactor_cmp_print(reactors[i], read_reactors[i]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "* UPDATE / READ REACTORS\n\n");

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        reactors[i].species = "U+G-PIES REACTOR " + std::to_string(i);
        randomizer.reactor_randomize(reactors[i]);

        db.update_reactor(reactors[i], &sqlite_code);
        fprintf(stdout, "REACTOR UPDATED\n");

        db.read_reactor(reactors[i].sqlite_id, read_reactors[i], &sqlite_code);
        fprintf(stdout, "READ REACTOR\t-\tsqlite code = %4d\n", sqlite_code);

        reactor_cmp_print(reactors[i], read_reactors[i]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "* DELETE REACTORS\n\n");

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        db.delete_reactor(reactors[i], &sqlite_code);
        fprintf(stdout, "* REACTOR \"%s\" DELETED\t-\tid = %4d\n", reactors[i].species.c_str(), reactors[i].sqlite_id);
    }

    fprintf(stdout, "\n");
}

void materials_crud()
{
    int sqlite_code = -1;

    // generate a list of randomized materials
    std::vector<Material> materials(VEC_SIZE, Material());
    for (int i = 0; i < VEC_SIZE; ++i)
    {
        materials[i].species = "G-PIES MATERIAL " + std::to_string(i);
        randomizer.material_randomize(materials[i]);

        db.create_material(materials[i], &sqlite_code);

        fprintf(stdout, "* CREATE %s\t-\tid = %4d\t-\tsqlite code = %4d\n",
            materials[i].species.c_str(), materials[i].sqlite_id, sqlite_code);
    }

    fprintf(stdout, "\n");

    std::vector<Material> read_materials;
    db.read_materials(read_materials, &sqlite_code);
    fprintf(stdout, "* READ MATERIALS - count = %4d - sqlite code = %4d\n" 
        "* EXISTING\t\t-\tREAD RESULT\n\n",
        (int)read_materials.size(), sqlite_code);

    Material* existing;
    Material* read;

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        material_cmp_print(materials[i], read_materials[i]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "* UPDATE / READ MATERIALS\n\n");

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        materials[i].species = "U+G-PIES MATERIAL " + std::to_string(i);
        randomizer.material_randomize(materials[i]);

        db.update_material(materials[i], &sqlite_code);
        fprintf(stdout, "MATERIAL UPDATED\n");

        db.read_material(materials[i].sqlite_id, read_materials[i], &sqlite_code);
        fprintf(stdout, "READ MATERIAL\t-\tsqlite code = %4d\n", sqlite_code);

        material_cmp_print(materials[i], read_materials[i]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "* DELETE MATERIALS\n\n");

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        db.delete_material(materials[i], &sqlite_code);
        fprintf(stdout, "* MATERIAL \"%s\" DELETED\t-\tid = %4d\n", materials[i].species.c_str(), materials[i].sqlite_id);
    }

    fprintf(stdout, "\n");
}

void reactor_cmp_print(NuclearReactor& r1, NuclearReactor& r2)
{
    fprintf(stdout, "%s\t-\t%s\n", r1.species.c_str(), r2.species.c_str());
    fprintf(stdout, "%s\t-\t%s\n", r1.creation_datetime.c_str(), r2.creation_datetime.c_str());
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.flux, r2.flux);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.temperature, r2.temperature);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.recombination, r2.recombination);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_bi, r2.i_bi);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_tri, r2.i_tri);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_quad, r2.i_quad);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_bi, r2.v_bi);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_tri, r2.v_tri);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_quad, r2.v_quad);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.dislocation_density_evolution, r2.dislocation_density_evolution);
}

void material_cmp_print(Material& r1, Material& r2)
{
    fprintf(stdout, "%s\t-\t%s\n", r1.species.c_str(), r2.species.c_str());
    fprintf(stdout, "%s\t-\t%s\n", r1.creation_datetime.c_str(), r2.creation_datetime.c_str());
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_migration, r2.i_migration);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_migration, r2.v_migration);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_diffusion_0, r2.i_diffusion_0);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_diffusion_0, r2.v_diffusion_0);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_formation, r2.i_formation);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_formation, r2.v_formation);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_binding, r2.i_binding);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_binding, r2.v_binding);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.recombination_radius, r2.recombination_radius);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_loop_bias, r2.i_loop_bias);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_dislocation_bias, r2.i_dislocation_bias);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.i_dislocation_bias_param, r2.i_dislocation_bias_param);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_loop_bias, r2.v_loop_bias);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_dislocation_bias, r2.v_dislocation_bias);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.v_dislocation_bias_param, r2.v_dislocation_bias_param);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.dislocation_density_0, r2.dislocation_density_0);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.grain_size, r2.grain_size);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.lattice_param, r2.lattice_param);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.burgers_vector, r2.burgers_vector);
    fprintf(stdout, "%g\t\t\t-\t%g\n", r1.atomic_volume, r2.atomic_volume);
}