#include <iostream>
#include <vector>

#include "client_db.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"
#include "gpies_exception.hpp"

#define VEC_SIZE 4

double randd();
void reactors_crud();
void reactor_randomize(NuclearReactor&);
void reactor_cmp_print(NuclearReactor&, NuclearReactor&);

ClientDb db;

int main(int argc, char* argv[])
{
    srand(std::time(nullptr));

    try
    {
        db.clear();
        db.init();

        std::cout << "\nREACTORS CRUD -----------------------------------\n";
        reactors_crud();
    }
    catch(const ClientDbException& e)
    {
        fprintf(stderr, "%s\n%s sqlite code = %4d\n\n", e.message.c_str(), e.sqlite_errmsg.c_str(), e.sqlite_code);
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
        reactor_randomize(reactors[i]);

        sqlite_code = db.create_reactor(reactors[i]);

        fprintf(stdout, "* CREATE %s\t-\tid = %4d\t-\tsqlite code = %4d\n",
            reactors[i].species.c_str(), reactors[i].sqlite_id, sqlite_code);
    }

    fprintf(stdout, "\n");

    std::vector<NuclearReactor> read_reactors;
    sqlite_code = db.read_reactors(read_reactors);
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

    int sqlite_changes = 0;
    for (int i = 0; i < VEC_SIZE; ++i)
    {
        reactors[i].species = "U+G-PIES REACTOR " + std::to_string(i);
        reactor_randomize(reactors[i]);

        sqlite_changes = db.update_reactor(reactors[i]);
        fprintf(stdout, "%d REACTOR(S) UPDATED\n", sqlite_changes);

        sqlite_code = db.read_reactor(reactors[i].sqlite_id, read_reactors[i]);
        fprintf(stdout, "READ REACTOR\t-\tsqlite code = %4d\n", sqlite_code);

        reactor_cmp_print(reactors[i], read_reactors[i]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "* DELETE REACTORS\n\n");

    for (int i = 0; i < VEC_SIZE; ++i)
    {
        sqlite_changes = db.delete_reactor(reactors[i]);
        fprintf(stdout, "* %4d REACTOR(S) DELETED\t-\tid = %4d\n",
            sqlite_changes, reactors[i].sqlite_id);
    }

    fprintf(stdout, "\n");
}

void reactor_randomize(NuclearReactor& reactor)
{
    reactor.flux = randd();
    reactor.temperature = randd();
    reactor.recombination = randd(); 
    reactor.i_bi = randd();
    reactor.i_tri = randd();
    reactor.i_quad = randd();
    reactor.v_bi = randd();
    reactor.v_tri = randd();
    reactor.v_quad = randd();
    reactor.dislocation_density_evolution = randd();
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