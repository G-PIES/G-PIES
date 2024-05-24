#include "parse.h"
#include "generator.h"

int main(int argc, char **argv)
{
    std::string filename = (argc < 2) ? "system.txt" : argv[1];
    std::ifstream system_src_file(filename, std::ios::in);
    SystemDeclarations system;
    read_system(system, system_src_file);
    system_src_file.close();

    std::ofstream outmodule("generated/system.h", std::ios::out);
    outmodule << "#include <cmath>\n"
              << "#include <nvector/nvector_serial.h>\n"
              << "#include <sstream>\n\n"
              << generate_constant_definitions(system) 
              << "\n\n" << generate_state_indices(system)
              << "\nconst size_t STATE_SIZE =" << system.next_index << ";"
              << "\n" << generate_function_declarations(system)
              << generate_summation_definitions(system)
              << generate_function_definitions(system)
              << generate_initial_state_setter(system)
              << generate_csv_getters(system)
              << generate_system(system);

    return 0;
}