#include <sqlite3.h>
#include <string>
#include <vector>

#include "client_db.hpp"
#include "db_queries.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"


// --------------------------------------------------------------------------------------------
// TEMPLATE FUNCTIONS 
// --------------------------------------------------------------------------------------------

template<typename T> int ClientDb::create_one(sqlite3_stmt* stmt, void (*err_callback)(sqlite3_stmt*, const T&), T& object)
{
    int sqlite_code;

    do
    {
        sqlite_code = sqlite3_step(stmt);
        if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);
    }
    while (SQLITE_DONE != sqlite_code);

    sqlite_code = last_insert_rowid(object.sqlite_id);
    if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);

    sqlite3_finalize(stmt);
    return sqlite_code;
}

template<typename T> int ClientDb::read_one(sqlite3_stmt* stmt, void (*row_callback)(sqlite3_stmt*, T&), void (*err_callback)(sqlite3_stmt*, const int), const int sqlite_id, T& object)
{
    int sqlite_code;

    sqlite_code = sqlite3_bind_int(stmt, 1, sqlite_id);
    if (is_sqlite_error(sqlite_code)) err_callback(stmt, sqlite_id);

    do
    {
        if (SQLITE_ROW == (sqlite_code = sqlite3_step(stmt)))
            row_callback(stmt, object);
        else if (is_sqlite_error(sqlite_code))
            err_callback(stmt, sqlite_id);
    }
    while (sqlite_code != SQLITE_DONE);

    sqlite3_finalize(stmt);
    return sqlite_code;
}

template<typename T> int ClientDb::read_all(sqlite3_stmt* stmt, void (*row_callback)(sqlite3_stmt*, T&), void (*err_callback)(sqlite3_stmt*), std::vector<T>& objects)
{
    int sqlite_code;

    do
    {
        if (SQLITE_ROW == (sqlite_code = sqlite3_step(stmt)))
        {
            T obj;
            row_callback(stmt, obj);
            objects.push_back(obj);
        }
        else if (is_sqlite_error(sqlite_code))
        {
            err_callback(stmt);
        }
    }
    while (sqlite_code != SQLITE_DONE);

    sqlite3_finalize(stmt);
    return sqlite_code;
}

template<typename T> int ClientDb::update_one(sqlite3_stmt* stmt, void (*err_callback)(sqlite3_stmt*, const T&), const T& object)
{
    int sqlite_code;

    do
    {
        sqlite_code = sqlite3_step(stmt);
        if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);
    } 
    while (SQLITE_DONE != sqlite_code);

    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

template<typename T> int ClientDb::delete_one(sqlite3_stmt* stmt, void (*err_callback)(sqlite3_stmt*, const T&), const T& object)
{
    int sqlite_code;
    sqlite3_bind_int(stmt, 1, object.sqlite_id);

    do
    {
        sqlite_code = sqlite3_step(stmt);
        if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);
    } 
    while (SQLITE_DONE != sqlite_code);

    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}


// --------------------------------------------------------------------------------------------
// COLUMN BINDING
// --------------------------------------------------------------------------------------------


void ClientDb::bind_reactor(sqlite3_stmt* stmt, const NuclearReactor& reactor)
{
    sqlite3_bind_text(stmt, 1, reactor.species.c_str(), reactor.species.length(), nullptr);
    sqlite3_bind_double(stmt, 2, reactor.flux);
    sqlite3_bind_double(stmt, 3, reactor.temperature);
    sqlite3_bind_double(stmt, 4, reactor.recombination);
    sqlite3_bind_double(stmt, 5, reactor.i_bi);
    sqlite3_bind_double(stmt, 6, reactor.i_tri);
    sqlite3_bind_double(stmt, 7, reactor.i_quad);
    sqlite3_bind_double(stmt, 8, reactor.v_bi);
    sqlite3_bind_double(stmt, 9, reactor.v_tri);
    sqlite3_bind_double(stmt, 10, reactor.v_quad);
    sqlite3_bind_double(stmt, 11, reactor.dislocation_density_evolution);
    if (is_valid_sqlite_id(reactor.sqlite_id)) sqlite3_bind_int(stmt, 12, reactor.sqlite_id);
}

void ClientDb::bind_material(sqlite3_stmt* stmt, const Material& material)
{
    sqlite3_bind_text(stmt, 1, material.species.c_str(), material.species.length(), nullptr);
    sqlite3_bind_double(stmt, 2, material.i_migration);
    sqlite3_bind_double(stmt, 3, material.v_migration);
    sqlite3_bind_double(stmt, 4, material.i_diffusion_0);
    sqlite3_bind_double(stmt, 5, material.v_diffusion_0);
    sqlite3_bind_double(stmt, 6, material.i_formation);
    sqlite3_bind_double(stmt, 7, material.v_formation);
    sqlite3_bind_double(stmt, 8, material.i_binding);
    sqlite3_bind_double(stmt, 9, material.v_binding);
    sqlite3_bind_double(stmt, 10, material.recombination_radius);
    sqlite3_bind_double(stmt, 11, material.i_loop_bias);
    sqlite3_bind_double(stmt, 12, material.i_dislocation_bias);
    sqlite3_bind_double(stmt, 13, material.i_dislocation_bias_param);
    sqlite3_bind_double(stmt, 14, material.v_loop_bias);
    sqlite3_bind_double(stmt, 15, material.v_dislocation_bias);
    sqlite3_bind_double(stmt, 16, material.v_dislocation_bias_param);
    sqlite3_bind_double(stmt, 17, material.dislocation_density_0);
    sqlite3_bind_double(stmt, 18, material.grain_size);
    sqlite3_bind_double(stmt, 19, material.lattice_param);
    sqlite3_bind_double(stmt, 20, material.burgers_vector);
    sqlite3_bind_double(stmt, 21, material.atomic_volume);
    if (is_valid_sqlite_id(material.sqlite_id)) sqlite3_bind_int(stmt, 22, material.sqlite_id);
}


// --------------------------------------------------------------------------------------------
// ROW CALLBACKS 
// --------------------------------------------------------------------------------------------


void ClientDb::row_read_reactor(sqlite3_stmt* stmt, NuclearReactor& reactor)
{
    reactor.sqlite_id = (int)sqlite3_column_int(stmt, 0);
    reactor.creation_datetime = (char*)sqlite3_column_text(stmt, 1);
    reactor.species = (char*)sqlite3_column_text(stmt, 2);
    reactor.flux = (double)sqlite3_column_double(stmt, 3);
    reactor.temperature = (double)sqlite3_column_double(stmt, 4);
    reactor.recombination = (double)sqlite3_column_double(stmt, 5);
    reactor.i_bi = (double)sqlite3_column_double(stmt, 6);
    reactor.i_tri = (double)sqlite3_column_double(stmt, 7);
    reactor.i_quad = (double)sqlite3_column_double(stmt, 8);
    reactor.v_bi = (double)sqlite3_column_double(stmt, 9);
    reactor.v_tri = (double)sqlite3_column_double(stmt, 10);
    reactor.v_quad = (double)sqlite3_column_double(stmt, 11);
    reactor.dislocation_density_evolution = (double)sqlite3_column_double(stmt, 12);
}

void ClientDb::row_read_material(sqlite3_stmt* stmt, Material& material)
{
    material.sqlite_id = (int)sqlite3_column_int(stmt, 0);
    material.creation_datetime = (char*)sqlite3_column_text(stmt, 1);
    material.species = (char*)sqlite3_column_text(stmt, 2);
    material.i_migration = (double)sqlite3_column_double(stmt, 3);
    material.v_migration = (double)sqlite3_column_double(stmt, 4);
    material.i_diffusion_0 = (double)sqlite3_column_double(stmt, 5);
    material.v_diffusion_0 = (double)sqlite3_column_double(stmt, 6);
    material.i_formation = (double)sqlite3_column_double(stmt, 7);
    material.v_formation = (double)sqlite3_column_double(stmt, 8);
    material.i_binding = (double)sqlite3_column_double(stmt, 9);
    material.v_binding = (double)sqlite3_column_double(stmt, 10);
    material.recombination_radius = (double)sqlite3_column_double(stmt, 11);
    material.i_loop_bias = (double)sqlite3_column_double(stmt, 12);
    material.i_dislocation_bias = (double)sqlite3_column_double(stmt, 13);
    material.i_dislocation_bias_param = (double)sqlite3_column_double(stmt, 14);
    material.v_loop_bias = (double)sqlite3_column_double(stmt, 15);
    material.v_dislocation_bias = (double)sqlite3_column_double(stmt, 16);
    material.v_dislocation_bias_param = (double)sqlite3_column_double(stmt, 17);
    material.dislocation_density_0 = (double)sqlite3_column_double(stmt, 18);
    material.grain_size = (double)sqlite3_column_double(stmt, 19);
    material.lattice_param = (double)sqlite3_column_double(stmt, 20);
    material.burgers_vector = (double)sqlite3_column_double(stmt, 21);
    material.atomic_volume = (double)sqlite3_column_double(stmt, 22);
}


// --------------------------------------------------------------------------------------------
// ERROR CALLBACKS 
// --------------------------------------------------------------------------------------------


void ClientDb::err_create_reactor(sqlite3_stmt* stmt, const NuclearReactor& reactor)
{
    std::string errmsg = "Failed to create reactor \"" + reactor.species + "\".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_read_reactors(sqlite3_stmt* stmt)
{
    sqlite3* db = sqlite3_db_handle(stmt);
    throw ClientDbException("Failed to read reactors.", sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_read_reactor(sqlite3_stmt* stmt, const int sqlite_id)
{
    std::string errmsg = "Failed to read reactor w/ id " + std::to_string(sqlite_id) + ".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_update_reactor(sqlite3_stmt* stmt, const NuclearReactor& reactor)
{
    std::string errmsg = "Failed to update reactor \"" +
        reactor.species + "\" w/ id " + std::to_string(reactor.sqlite_id) + ".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_delete_reactor(sqlite3_stmt* stmt, const NuclearReactor& reactor)
{
    std::string errmsg = "Failed to delete reactor \"" +
        reactor.species + "\" w/ id " + std::to_string(reactor.sqlite_id) + ".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_create_material(sqlite3_stmt* stmt, const Material& material)
{
    std::string errmsg = "Failed to create material \"" + material.species + "\".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_read_materials(sqlite3_stmt* stmt)
{
    sqlite3* db = sqlite3_db_handle(stmt);
    throw ClientDbException("Failed to read materials.", sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_read_material(sqlite3_stmt* stmt, const int sqlite_id)
{
    std::string errmsg = "Failed to read material w/ id " + std::to_string(sqlite_id) + ".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_update_material(sqlite3_stmt* stmt, const Material& material)
{
    std::string errmsg = "Failed to update material \"" +
        material.species + "\" w/ id " + std::to_string(material.sqlite_id) + ".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDb::err_delete_material(sqlite3_stmt* stmt, const Material& material)
{
    std::string errmsg = "Failed to delete material \"" +
        material.species + "\" w/ id " + std::to_string(material.sqlite_id) + ".";
    sqlite3* db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db), sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}


// --------------------------------------------------------------------------------------------
// UTILITIES 
// --------------------------------------------------------------------------------------------


int ClientDb::last_insert_rowid(int& sqlite_id)
{
    if (!db) return -1;

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::last_insert_rowid.c_str(), db_queries::last_insert_rowid.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to retrieve database row id.", sqlite3_errmsg(db), sqlite3_errcode(db));

    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to retrieve database row id.", sqlite3_errmsg(db), sqlite3_errcode(db));

    sqlite_id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return sqlite_code;
}


// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  PUBLIC INTERFACE FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------


bool ClientDb::init(int* sqlite_result_code)
{
    int sqlite_code;
    char* sqlite_errmsg;
    sqlite_code = sqlite3_initialize();
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to initialize database.", sqlite3_errmsg(db), sqlite_code);

    if (!db) open();

    sqlite_code = sqlite3_exec(db, db_queries::init.c_str(), nullptr, nullptr, &sqlite_errmsg);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to initialize database.", sqlite_errmsg, sqlite_code, db_queries::init);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::clear(int* sqlite_result_code)
{
    if (!db) open();

    int sqlite_code;
    char* sqlite_errmsg;

    sqlite_code = sqlite3_exec(db, db_queries::clear.c_str(), nullptr, nullptr, nullptr);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to clear database.", sqlite_errmsg, sqlite_code, db_queries::clear);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::create_reactor(NuclearReactor& reactor, int* sqlite_result_code)
{
    if (is_valid_sqlite_id(reactor.sqlite_id))
        throw ClientDbException("Failed to create reactor, it already exists.");
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::create_reactor.c_str(), db_queries::create_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_create_reactor(stmt, reactor);

    bind_reactor(stmt, reactor);

    sqlite_code = create_one<NuclearReactor>(stmt, err_create_reactor, reactor);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_reactors(std::vector<NuclearReactor>& reactors, int* sqlite_result_code)
{
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::read_reactors.c_str(), db_queries::read_reactors.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_read_reactors(stmt);

    sqlite_code = read_all<NuclearReactor>(stmt, row_read_reactor, err_read_reactors, reactors);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_reactor(const int sqlite_id, NuclearReactor& reactor, int* sqlite_result_code)
{
    if (!is_valid_sqlite_id(sqlite_id)) 
        throw ClientDbException("Failed to read reactor. Invalid id.");

    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::read_reactor.c_str(), db_queries::read_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_read_reactor(stmt, sqlite_id);

    sqlite_code = read_one<NuclearReactor>(stmt, row_read_reactor, err_read_reactor, sqlite_id, reactor);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code) && is_valid_sqlite_id(reactor.sqlite_id);
}

bool ClientDb::update_reactor(const NuclearReactor& reactor, int* sqlite_result_code)
{
    if (!is_valid_sqlite_id(reactor.sqlite_id))
        throw ClientDbException("Failed to update reactor. Invalid id.");

    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::update_reactor.c_str(), db_queries::update_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_update_reactor(stmt, reactor);

    bind_reactor(stmt, reactor);

    sqlite_code = update_one<NuclearReactor>(stmt, err_update_reactor, reactor);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::delete_reactor(const NuclearReactor& reactor, int* sqlite_result_code)
{
    if (!is_valid_sqlite_id(reactor.sqlite_id))
        throw ClientDbException("Failed to delete reactor. Invalid id.");

    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::delete_reactor.c_str(), db_queries::delete_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_delete_reactor(stmt, reactor);

    sqlite_code = delete_one<NuclearReactor>(stmt, err_delete_reactor, reactor);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::create_material(Material& material, int* sqlite_result_code)
{
    if (is_valid_sqlite_id(material.sqlite_id))
        throw ClientDbException("Failed to create material, it already exists.");
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::create_material.c_str(), db_queries::create_material.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_create_material(stmt, material);

    bind_material(stmt, material);

    sqlite_code = create_one<Material>(stmt, err_create_material, material);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_materials(std::vector<Material>& materials, int* sqlite_result_code)
{
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::read_materials.c_str(), db_queries::read_materials.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_read_materials(stmt);

    sqlite_code = read_all<Material>(stmt, row_read_material, err_read_materials, materials);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_material(const int sqlite_id, Material& material, int* sqlite_result_code)
{
    if (!is_valid_sqlite_id(sqlite_id)) 
        throw ClientDbException("Failed to read material. Invalid id.");

    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::read_material.c_str(), db_queries::read_material.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_read_material(stmt, sqlite_id);

    sqlite_code = read_one<Material>(stmt, row_read_material, err_read_material, sqlite_id, material);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code) && is_valid_sqlite_id(material.sqlite_id);
}

bool ClientDb::update_material(const Material& material, int* sqlite_result_code)
{
    if (!is_valid_sqlite_id(material.sqlite_id))
        throw ClientDbException("Failed to update material. Invalid id.");

    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::update_material.c_str(), db_queries::update_material.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_update_material(stmt, material);

    bind_material(stmt, material);

    sqlite_code = update_one<Material>(stmt, err_update_material, material);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::delete_material(const Material& material, int* sqlite_result_code)
{
    if (!is_valid_sqlite_id(material.sqlite_id))
        throw ClientDbException("Failed to delete material. Invalid id.");

    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::delete_material.c_str(), db_queries::delete_material.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_delete_material(stmt, material);

    sqlite_code = delete_one<Material>(stmt, err_delete_material, material);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::open(int* sqlite_result_code)
{
    int sqlite_code;

    sqlite_code = sqlite3_open(path.c_str(), &db);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to open local database.", sqlite3_errmsg(db), sqlite_code);

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::close(int* sqlite_result_code)
{
    if (!db) return 0;

    int sqlite_code;

    sqlite_code = sqlite3_close(db);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to close database.", sqlite3_errmsg(db), sqlite_code);
    else db = nullptr;

    if (sqlite_result_code) *sqlite_result_code = sqlite_code;
    return is_sqlite_success(sqlite_code);
}

bool ClientDb::is_open()
{
    return db;
}

bool ClientDb::is_sqlite_success(const int sqlite_code)
{
    // sqlite3 error handling: https://www.sqlite.org/rescode.html
    return SQLITE_OK == sqlite_code ||
        SQLITE_ROW == sqlite_code ||
        SQLITE_DONE == sqlite_code;
}

bool ClientDb::is_sqlite_error(const int sqlite_code)
{
    // sqlite3 error handling: https://www.sqlite.org/rescode.html
    return SQLITE_OK != sqlite_code &&
        SQLITE_ROW != sqlite_code &&
        SQLITE_DONE != sqlite_code;
}

bool ClientDb::is_valid_sqlite_id(const int sqlite_id)
{
    return 0 < sqlite_id;
}

ClientDb::ClientDb(const char* db_path, const bool lazy)
: path(db_path)
{
    if (lazy) db = nullptr;
    else open();
}

ClientDb::~ClientDb()
{
    close();
    sqlite3_shutdown();
}
