#include <sqlite3.h>
#include <string>
#include <vector>

#include "client_db.hpp"
#include "db_queries.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

int ClientDb::init()
{
    int sqlite_code;
    char* sqlite_errmsg;
    sqlite_code = sqlite3_initialize();
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to initialize database.", sqlite_code);

    if (!db) open();

    sqlite_code = sqlite3_exec(db, db_queries::init.c_str(), nullptr, nullptr, &sqlite_errmsg);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to initialize database.", sqlite_code, sqlite_errmsg);

    return sqlite_code;
}

int ClientDb::clear()
{
    if (!db) open();

    int sqlite_code;
    char* sqlite_errmsg;

    sqlite_code = sqlite3_exec(db, db_queries::clear.c_str(), nullptr, nullptr, nullptr);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to clear database.", sqlite_code, sqlite_errmsg);

    return sqlite_code;
}

int ClientDb::create_reactor(NuclearReactor& reactor)
{
    if (reactor.sqlite_id > 0)
        throw ClientDbException("Failed to create reactor, it already exists.");
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::create_reactor.c_str(), db_queries::create_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_create_reactor(db, reactor);

    bind_reactor(stmt, reactor);

    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code)) err_create_reactor(db, reactor);
    sqlite3_finalize(stmt);

    sqlite_code = last_insert_rowid(reactor.sqlite_id);

    return sqlite_code;
}

int ClientDb::read_reactors(std::vector<NuclearReactor>& reactors)
{
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::read_reactors.c_str(), db_queries::read_reactors.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_read_reactors(db);

    sqlite_code = read<NuclearReactor>(stmt, row_read_reactor, err_read_reactors, reactors);

    return sqlite_code;
}

int ClientDb::read_reactor(const int id, NuclearReactor& reactor)
{
    if (id < 0) return -1;
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::read_reactor.c_str(), db_queries::read_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_read_reactor(db, id);

    sqlite_code = read<NuclearReactor>(stmt, row_read_reactor, err_read_reactor, id, reactor);

    return sqlite_code;
}

int ClientDb::update_reactor(const NuclearReactor& reactor)
{
    if (reactor.sqlite_id < 0) return 0;
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::update_reactor.c_str(), db_queries::update_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_update_reactor(db, reactor.sqlite_id);

    bind_reactor(stmt, reactor);

    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code)) err_update_reactor(db, reactor.sqlite_id);

    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int ClientDb::delete_reactor(const int id)
{
    if (id < 0) return 0;
    if (!db) open();

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::update_reactor.c_str(), db_queries::update_reactor.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code)) err_delete_reactor(db, id);

    sqlite3_bind_int(stmt, 0, id);

    do
    {
        sqlite_code = sqlite3_step(stmt);
        if (is_sqlite_error(sqlite_code)) err_delete_reactor(db, id);
    } 
    while (SQLITE_DONE != sqlite_code);

    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

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
    if (reactor.sqlite_id > 0) sqlite3_bind_int(stmt, 12, reactor.sqlite_id);
}

template<typename T> int ClientDb::read(sqlite3_stmt* stmt, void (*row_callback)(sqlite3_stmt*, T&), void (*err_callback)(sqlite3*, const int), const int id, T& object)
{
    int sqlite_code;

    sqlite3_bind_int(stmt, 0, id);
    if (is_sqlite_error(sqlite_code)) err_callback(db, id);

    do
    {
        if (SQLITE_ROW == (sqlite_code = sqlite3_step(stmt)))
            row_callback(stmt, object);
        else if (is_sqlite_error(sqlite_code))
            err_callback(db, id);
    }
    while (sqlite_code != SQLITE_DONE);

    sqlite3_finalize(stmt);
    return sqlite_code;
}

template<typename T> int ClientDb::read(sqlite3_stmt* stmt, void (*row_callback)(sqlite3_stmt*, T&), void (*err_callback)(sqlite3*), std::vector<T>& objects)
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
            err_callback(db);
        }
    }
    while (sqlite_code != SQLITE_DONE);

    sqlite3_finalize(stmt);
    return sqlite_code;
}

int ClientDb::open()
{
    int sqlite_code;

    sqlite_code = sqlite3_open(path.c_str(), &db);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to open local database.", sqlite_code);

    return sqlite_code;
}

int ClientDb::close()
{
    if (!db) return 0;

    int sqlite_code;

    sqlite_code = sqlite3_close(db);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to close database.", sqlite_code);
    else db = nullptr;

    return sqlite_code;
}

bool ClientDb::is_open()
{
    return db;
}

bool ClientDb::is_sqlite_error(int sqlite_code)
{
    // sqlite3 error handling: https://www.sqlite.org/rescode.html
    return SQLITE_OK != sqlite_code &&
        SQLITE_ROW != sqlite_code && 
        SQLITE_DONE != sqlite_code;
}

int ClientDb::last_insert_rowid(int& id)
{
    if (!db) return -1;

    int sqlite_code;
    sqlite3_stmt* stmt;

    sqlite_code = sqlite3_prepare_v2(db, db_queries::last_insert_rowid.c_str(), db_queries::last_insert_rowid.size(), &stmt, nullptr);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to retrieve database row id.", sqlite3_errcode(db), sqlite3_errmsg(db));

    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code))
        throw ClientDbException("Failed to retrieve database row id.", sqlite3_errcode(db), sqlite3_errmsg(db));

    id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return sqlite_code;
}


// --------------------------------------------------------------------------------------------
// ROW CALLBACKS 
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
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
// ERROR CALLBACKS 
void ClientDb::err_create_reactor(sqlite3* db, const NuclearReactor& reactor)
{
    std::string errmsg = "Failed to create reactor \"";
    errmsg.append(reactor.species);
    errmsg += "\".";

    throw ClientDbException(errmsg.c_str(), sqlite3_errcode(db), sqlite3_errmsg(db));
}

void ClientDb::err_read_reactors(sqlite3* db)
{
    throw ClientDbException("Failed to read reactors.", sqlite3_errcode(db), sqlite3_errmsg(db));
}

void ClientDb::err_read_reactor(sqlite3* db, const int id)
{
    std::string errmsg = "Failed to read reactor w/ id=" + std::to_string(id) + ".";

    throw ClientDbException(errmsg.c_str(), sqlite3_errcode(db), sqlite3_errmsg(db));
}

void ClientDb::err_update_reactor(sqlite3* db, const int id)
{
    std::string errmsg = "Failed to update reactor w/ id=" + std::to_string(id) + ".";

    throw ClientDbException(errmsg.c_str(), sqlite3_errcode(db), sqlite3_errmsg(db));
}

void ClientDb::err_delete_reactor(sqlite3* db, const int id)
{
    std::string errmsg = "Failed to delete reactor w/ id=" + std::to_string(id) + ".";

    throw ClientDbException(errmsg.c_str(), sqlite3_errcode(db), sqlite3_errmsg(db));
}
// --------------------------------------------------------------------------------------------


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