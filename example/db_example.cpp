#include <iostream>

#include "client_db.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"
#include "gpies_exception.hpp"

int main(int argc, char* argv[])
{
    ClientDb db;

    try
    {
        db.clear();
        db.init();

        for (int i = 0; i < 10; ++i)
        {
            NuclearReactor reactor;
            nuclear_reactors::OSIRIS(reactor);
            reactor.species += " " + std::to_string(i);
            db.create_reactor(reactor);

            std::cout << reactor.species << " created w/ SQLite id "
                << reactor.sqlite_id << " @ " << reactor.creation_datetime << '\n';
        }
    }
    catch(const ClientDbException& e)
    {
        std::cerr << e.message << '\n' << e.sqlite_errmsg << " SQLite code : " << e.sqlite_code;
    }

    return 0;
}