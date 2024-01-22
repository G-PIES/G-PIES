#ifndef CLIENT_DB_HPP
#define CLIENT_DB_HPP


#include <vector>
#include <sqlite3.h>
#include "db_queries.hpp"
#include "gpies_exception.hpp"

const char* DEFAULT_CLIENT_DB_PATH = "../db/gpies.db";

class ClientDb
{
    public:
    int init();
    int clear();

    // reactor CRUD
    int create_reactor(const NuclearReactor&);
    int read_reactors(const std::vector<NuclearReactor>&);
    int read_reactor(const int, NuclearReactor&);
    int update_reactor(const NuclearReactor&);
    int delete_reactor(const int);

    // material CRUD
    int create_material(const Material&);
    int read_materials(const std::vector<Material>&);
    int read_material(const int, Material&);
    int update_material(const Material&);
    int delete_material(const int);

    // simulation CRUD

    int open();
    int close();

    static bool is_sqlite_error(int);

    ClientDb(const char*, const bool);
    ~ClientDb();

    private:
    sqlite3* db;

    // --------------------------------------------------------------------------------------------
    // READ 
    template<typename T> int read(sqlite3_stmt*, void (*)(sqlite3_stmt*, T&), void (*)(sqlite3*, const int), T&);
    template<typename T> int read(sqlite3_stmt*, void (*)(sqlite3_stmt*, T&), void (*)(sqlite3*), const std::vector<T>&);
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // COLUMN BINDING
    static void bind_reactor(sqlite3_stmt*, const NuclearReactor&);
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // ROW CALLBACKS 
    static void row_read_reactor(sqlite3_stmt*, NuclearReactor&);
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // ERROR CALLBACKS 
    static void err_create_reactor(sqlite3*, const NuclearReactor&);
    static void err_read_reactors(sqlite3*);
    static void err_read_reactor(sqlite3*, const int);
    void err_update_reactor(sqlite3*, const int);
    void err_delete_reactor(sqlite3*, const int);
    // --------------------------------------------------------------------------------------------

    const char* path;
};

class ClientDbException : GpiesException
{
    public:
    ClientDbException::ClientDbException(const char* message, const int sqlite_code, const char* sqlite_errmsg = nullptr)
    : GpiesException(message), sqlite_code(sqlite_code), sqlite_errmsg(sqlite_errmsg)
    {
    }

    int sqlite_code;
    const char* sqlite_errmsg;
};


#endif // CLIENT_DB_HPP