#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

#include "expression.h"
#include "parse.h"

std::string generate_index_range(SystemDeclarations &system, Symbol state_symbol);
std::string generate_setter_list(SystemDeclarations &system, InitialState &initial_state);
std::string generate_derivative_list(SystemDeclarations &system, StateVariable &state_variable);
std::string generate_csv_list(SystemDeclarations &system, Symbol state_symbol);
std::string generate_constant_definitions(SystemDeclarations &system);
std::string generate_function_declarations(SystemDeclarations &system);
std::string generate_function_definitions(SystemDeclarations &system);
std::string generate_state_indices(SystemDeclarations &system);
std::string generate_summation_definitions(SystemDeclarations& system);
std::string generate_initial_state_setter(SystemDeclarations &system);
std::string generate_derivative_definitions(SystemDeclarations &system);
std::string generate_csv_getters(SystemDeclarations &system);
std::string generate_system(SystemDeclarations &system);