#ifndef CLIENT_DB_HPP
#define CLIENT_DB_HPP

#include <memory>
#include <string>
#include <vector>

#include "model/history_simulation.hpp"
#include "utils/gpies_exception.hpp"
#include "utils/types.hpp"

struct NuclearReactor;
struct Material;

static const std::string DB_NAME = "gpies.db";
static const std::string DEV_DEFAULT_CLIENT_DB_PATH = "./db";

class ClientDbImpl;

class ClientDb {
 private:
  std::unique_ptr<ClientDbImpl> _impl;

 public:
  // Initializes and opens the SQLite database, followed by any needed schema
  // maintenance. |sqlite_code| can optionally be retrieved. Returns true on
  // success.
  bool init(int *sqlite_code = nullptr);

  // Drops all tables in the SQLite database.
  // *** WARNING *** All data will be lost.
  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool clear(int *sqlite_code = nullptr);

  // --------------------------------------------------------------------------------------------
  // REACTOR CRUD

  // Creates a reactor in the local database, assigning a value to
  // NuclearReactor.sqlite_id. |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool create_reactor(NuclearReactor &reactor, int *sqlite_code = nullptr,
                      bool is_preset = true);

  // Reads all reactors from the local database, populating |reactors|.
  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool read_reactors(std::vector<NuclearReactor> &reactors,
                     int *sqlite_code = nullptr);

  // Attempts to read a single |reactor| from the local database, matching the
  // specified |sqlite_id|. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool read_reactor(const int sqlite_id, NuclearReactor &reactor,
                    int *sqlite_code = nullptr);

  // Attempts to update a reactor in the local database, matching to
  // |reactor.sqlite_id|. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool update_reactor(const NuclearReactor &reactor,
                      int *sqlite_code = nullptr);

  // Attempts to delete a reactor in the local database, matching to
  // |reactor.sqlite_id|. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool delete_reactor(const NuclearReactor &reactor,
                      int *sqlite_code = nullptr);

  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  // MATERIAL CRUD

  // Creates a material in the local database, assigning a value to
  // Material.sqlite_id. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool create_material(Material &material, int *sqlite_code = nullptr,
                       bool is_preset = true);

  // Reads all materials from the local database, populating |materials|.
  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool read_materials(std::vector<Material> &materials,
                      int *sqlite_code = nullptr);

  // Attempts to read a single |material| from the local database, matching
  // the specified |sqlite_id|. |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool read_material(const int sqlite_id, Material &material,
                     int *sqlite_code = nullptr);

  // Attempts to update a material in the local database, matching to
  // |material.sqlite_id|. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool update_material(const Material &material, int *sqlite_code = nullptr);

  // Attempts to delete a material in the local database, matching to
  // |material.sqlite_id|. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool delete_material(const Material &material, int *sqlite_code = nullptr);

  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  // SIMULATION CRUD

  // Creates a simulation in the local database, assigning a value to
  // simulation.sqlite_id. |sqlite_code| can optionally be retrieved. Returns
  // true on success.
  bool create_simulation(HistorySimulation &simulation,
                         int *sqlite_code = nullptr);

  // Reads all simulations from the local database, populating |simulations|.
  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool read_simulations(std::vector<HistorySimulation> &simulations,
                        int *sqlite_code = nullptr);

  // Attempts to read a single |simulation| from the local database, matching
  // the specified |sqlite_id|. |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool read_simulation(const int sqlite_id, HistorySimulation &simulation,
                       int *sqlite_code = nullptr);

  // Attempts to delete a simulation in the local database, matching to
  // |simulation.sqlite_id|. |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool delete_simulation(const HistorySimulation &simulation,
                         int *sqlite_code = nullptr);

  // Attempts to delete all simulations in the local database.
  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool delete_simulations(int *sqlite_code = nullptr);

  // --------------------------------------------------------------------------------------------

  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool open(int *sqlite_code = nullptr);

  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool close(int *sqlite_code = nullptr);

  // Returns true if the SQLite database is currently open.
  bool is_open();

  // Returns true if |sqlite_code| reprents a successful result.
  // https://www.sqlite.org/rescode.html
  static bool is_sqlite_success(const int sqlite_code);

  // Returns true if |sqlite_code| reprents a successful result.
  // https://www.sqlite.org/rescode.html
  static bool is_sqlite_error(const int sqlite_code);

  // Returns true if |sqlite_id| is a value primary key value.
  // NOTE: This does NOT check if |sqlite_id| exists within the database.
  static bool is_valid_sqlite_id(const int sqlite_id);

  // Returns the number of rows effected by the latest query.
  int changes();

  // Specify the |db_path| where the SQLite database resides
  // |lazy| will wait to open the database until |init| is called.
  ClientDb(const std::string &db_path = DEV_DEFAULT_CLIENT_DB_PATH,
           const bool lazy = true);
  ~ClientDb();
};

class ClientDbException : public GpiesException {
 public:
  ClientDbException(const std::string &message,
                    const std::string &sqlite_errmsg = "",
                    const int sqlite_code = -1, const std::string &query = "")
      : GpiesException(message),
        sqlite_errmsg(sqlite_errmsg),
        sqlite_code(sqlite_code),
        query(query) {}

  std::string sqlite_errmsg;
  int sqlite_code;
  std::string query;
};

#endif  // CLIENT_DB_HPP
