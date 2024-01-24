#ifndef CLIENT_DB_HPP
#define CLIENT_DB_HPP

#include <vector>
#include <sqlite3.h>
#include "gpies_exception.hpp"

class NuclearReactor;
class Material;

static const char* DEFAULT_CLIENT_DB_PATH = "./db/gpies.db";

class ClientDb
{
    public:
    bool init(int* = nullptr);
    bool clear(int* = nullptr);

    // reactor CRUD
    bool create_reactor(NuclearReactor&, int* = nullptr);
    bool read_reactors(std::vector<NuclearReactor>&, int* = nullptr);
    bool read_reactor(const int, NuclearReactor&, int* = nullptr);
    bool update_reactor(const NuclearReactor&, int* = nullptr);
    bool delete_reactor(const NuclearReactor&, int* = nullptr);

    // material CRUD

    // simulation CRUD

    bool open(int* = nullptr);
    bool close(int* = nullptr);

    bool is_open();
    static bool is_sqlite_success(const int);
    static bool is_sqlite_error(const int);
    static bool is_valid_sqlite_id(const int);

    ClientDb(const char* = DEFAULT_CLIENT_DB_PATH, const bool = true);
    ~ClientDb();

    private:
    sqlite3* db;
    std::string path;

    int last_insert_rowid(int&);

    // --------------------------------------------------------------------------------------------
    // CREATE
    template<typename T> int create_one(sqlite3_stmt*, void (*)(sqlite3_stmt*, const T&), T&);
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // READ 
    template<typename T> int read_one(sqlite3_stmt*, void (*)(sqlite3_stmt*, T&), void (*)(sqlite3_stmt*, const int), const int, T&);
    template<typename T> int read_all(sqlite3_stmt*, void (*)(sqlite3_stmt*, T&), void (*)(sqlite3_stmt*), std::vector<T>&);
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // UPDATE / DELETE
    template<typename T> int update_one(sqlite3_stmt*, void (*)(sqlite3_stmt*, const T&), const T&);
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // DELETE 
    template<typename T> int delete_one(sqlite3_stmt*, void (*)(sqlite3_stmt*, const T&), const T&);
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
    static void err_create_reactor(sqlite3_stmt*, const NuclearReactor&);
    static void err_read_reactors(sqlite3_stmt*);
    static void err_read_reactor(sqlite3_stmt*, const int);
    static void err_update_reactor(sqlite3_stmt*, const NuclearReactor&);
    static void err_delete_reactor(sqlite3_stmt*, const NuclearReactor&);
    // --------------------------------------------------------------------------------------------
};

class ClientDbException : public GpiesException
{
    public:
    ClientDbException(const std::string& message, const std::string& sqlite_errmsg = "", const int sqlite_code = -1, const std::string& query = "")
    : GpiesException(message), sqlite_errmsg(sqlite_errmsg), sqlite_code(sqlite_code), query(query)
    {
    }

    std::string sqlite_errmsg;
    int sqlite_code;
    std::string query;
};

#endif // CLIENT_DB_HPP