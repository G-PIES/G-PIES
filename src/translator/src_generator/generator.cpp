#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

#include "expression.h"
#include "parse.h"

std::string generate_index_range(SystemDeclarations &system, Symbol state_symbol)
{
    for (auto p : state_symbol.parameters) // Skip constrained definitions - only the catchall needs indices
    {
        if (p.type != ParameterType::VARIABLE)
        {
            return "";
        }
    }

    auto &range_symbol = state_symbol.parameters[0].symbol.value();
    if (!system.ranges.count(range_symbol))
    {
        std::cerr << "Error: Tried to generate indices for a list " << range_symbol << " which doesn't exist.\n";
        return "";
    }

    std::stringstream str;

    auto range = system.ranges[range_symbol];
    str << "const size_t INDEX_" << state_symbol.to_string() << "_START = " << system.next_index << ";\n";
    str << "const size_t INDEX_" << state_symbol.to_string() << "_SIZE = " << range.end->generate(system) << " - " << range.start->generate(system) << " + 1;\n";
    system.next_index = "INDEX_" + state_symbol.to_string() + "_START + INDEX_" + state_symbol.to_string() + "_SIZE";

    return str.str();
}

std::string generate_setter_list(SystemDeclarations &system, InitialState &initial_state)
{
    if (initial_state.symbol.parameters[0].type != ParameterType::VARIABLE)
    {
        std::cerr << "Error: Trying to generate indices for a non variable list parameter." << std::endl;
        return "";
    }

    auto &range_symbol = initial_state.symbol.parameters[0].symbol.value();
    if (!system.ranges.count(range_symbol))
    {
        std::cerr << "Error: Tried to generate setters for a list which doesn't exist.\n";
        return "";
    }

    auto range = system.ranges[range_symbol];
    system.bound_parameters.clear();
    system.bound_parameters[range_symbol] = true;

    std::stringstream str;

    str << "\n    for (size_t " << range_symbol << " = " << range.start->generate(system) << "; "
        << range_symbol << " <= " << range.end->generate(system) << "; "
        << "++" << range_symbol << ")\n"
        << "    {\n"

        << "        values[INDEX_" << initial_state.symbol.to_string() << "_START + (" << range_symbol << " - 1)] = "
        << initial_state.rhs->generate(system) << ";\n"

        << "    }\n";

    return str.str();
}

std::string generate_derivative_list(SystemDeclarations &system, StateVariable &state_variable)
{
    for (auto p : state_variable.symbol.parameters) // Skip constrained definitions - they're handled in a separate pass
    {
        if (p.type != ParameterType::VARIABLE)
        {
            return "";
        }
    }
    
    if (state_variable.symbol.parameters[0].type != ParameterType::VARIABLE)
    {
        std::cerr << "Warning: Trying to generate indices for a non variable list parameter." << std::endl;
        return "";
    }

    auto &range_symbol = state_variable.symbol.parameters[0].symbol.value();
    if (!system.ranges.count(range_symbol))
    {
        std::cerr << "Error: Tried to generate derivatives for a list which doesn't exist.\n";
        return "";
    }

    auto range = system.ranges[range_symbol];
    system.bound_parameters.clear();
    system.bound_parameters[range_symbol] = true;

    std::stringstream str;

    str << "\n    for (size_t " << range_symbol << " = " << range.start->generate(system) << "; "
        << range_symbol << " <= " << range.end->generate(system) << "; "
        << "++" << range_symbol << ")\n"
        << "    {\n"

        << "        derivatives[INDEX_" << state_variable.symbol.to_string() << "_START + (" << range_symbol << " - 1)] = "
        << state_variable.rhs->generate(system) << ";\n"

        << "    }\n";

    return str.str();
}

std::string generate_csv_list(SystemDeclarations &system, Symbol state_symbol)
{
    for (auto p : state_symbol.parameters) // Skip constrained definitions - catchall provides all labels
    {
        if (p.type != ParameterType::VARIABLE)
        {
            return "";
        }
    }

    auto &range_symbol = state_symbol.parameters[0].symbol.value();
    if (!system.ranges.count(range_symbol))
    {
        std::cerr << "Error: Tried to generate csv labels for a list which doesn't exist.\n";
        return "";
    }

    std::stringstream str;

    auto range = system.ranges[range_symbol];
    str << "\n\tfor (size_t i = " << range.start->generate(system) << "; i <= " << range.end->generate(system)  << "; ++i)";
    str << "\n\t{";
    str << "\n\t\tstr << \", " << state_symbol.to_string() << "[\" << i << \"]\";";
    str << "\n\t}\n";

    return str.str();
}

std::string generate_constant_definitions(SystemDeclarations &system)
{
    std::stringstream str;

    for (auto &f : system.function_definitions)
    {
        if (!f.is_constant(system))
            continue;

        if (f.definitions.size() != 1)
        {
            std::cerr << "Error: Constant " << f.symbol.to_string() << " must have 1 and only 1 definition.\n";
            return "0";
        }

        str << "\ndouble " << f.symbol.to_string() << " = " << f.definitions[0].expression->generate(system) << ";";
    }

    return str.str();
}

std::string generate_function_declarations(SystemDeclarations &system)
{
    std::stringstream str;

    auto &functions = system.function_definitions;

    for (auto &f : functions)
    {
        if (f.is_constant(system))
            continue;

        auto main_definition = f.get_catchall_definition();

        str << "\ndouble " << f.symbol.to_string() << "(";
        for (auto i = 0; i < main_definition.parameters.size(); ++i)
        {
            auto &p = main_definition.parameters[i];
            if (p.type != ParameterType::VARIABLE)
            {
                std::cerr << "Error: Main definition of a function must only have variable parameters.\n";
                continue;
            }
            str << (i != 0 ? ", " : "") << "double " << p.symbol.value();
        }

        if (f.is_state_dependent(system))
        {
            str << (f.symbol.parameters.size() > 0 ? ", " : "") << "double* values";
        }

        str << ");";
    }

    return str.str();
}

std::string generate_function_definitions(SystemDeclarations &system)
{
    std::stringstream str;

    auto &functions = system.function_definitions;
    for (auto &f : functions)
    {
        if (f.is_constant(system))
            continue;

        auto name = f.symbol.to_string();
        if (f.definitions.size() == 0)
        {
            std::cerr << "Error: " << name << " is missing definition.\n";
            continue;
        }

        auto main_definition = f.get_catchall_definition();

        str << "\n\ndouble " << name << "(";
        system.bound_parameters.clear();
        for (auto i = 0; i < main_definition.parameters.size(); ++i)
        {
            auto p = main_definition.parameters[i];
            if (p.type != ParameterType::VARIABLE)
            {
                std::cerr << "Error: Main definition of a function must only have variable parameters.\n";
                continue;
            }
            str << (i != 0 ? ", " : "") << "double " << p.symbol.value();
            system.bound_parameters[p.symbol.value()] = true;
        }

        if (f.is_state_dependent(system))
        {
            str << (f.symbol.parameters.size() > 0 ? ", " : "") << "double* values";
        }

        str << ")"
            << "\n{";

        for (auto definition : f.definitions)
        {
            if (definition.is_catchall())
                continue;

            str << "\n\tif (" << definition.get_parameter_constraints(system, main_definition) << ") {"
                << "\n\t\t return " << definition.expression->generate(system) << ";"
                << "\n\t}";
        }

        str << "\n\treturn " << main_definition.expression->generate(system) << ";\n"
            << "}";
    }

    return str.str();
}

std::string generate_state_indices(SystemDeclarations &system)
{
    std::stringstream str;
    auto &state_variables = system.state_variables;

    for (size_t i = 0; i < state_variables.size(); ++i)
    {
        if (state_variables[i].symbol.is_list())
        {
            str << generate_index_range(system, state_variables[i].symbol);
        }
        else
        {
            str << "\nconst size_t INDEX_" << state_variables[i].symbol.to_string() << " = " << system.next_index << ";";
            system.next_index = "INDEX_" + state_variables[i].symbol.to_string() + " + 1";
        }
    }

    return str.str();
}

std::string generate_summation_definitions(SystemDeclarations& system)
{
    std::stringstream str;

    for (auto summation : system.summation_definitions)
    {
        system.bound_parameters[summation.index.name] = true;
        str << "\n\ndouble " << summation.symbol.to_string() << "(double* values) {"
            << "\n\tdouble sum = 0.0;"
            << "\n\tfor (size_t " << summation.index.to_string() << " = " << summation.range.start->generate(system) << "; "
            << summation.index.to_string() << " < " << summation.range.end->generate(system) << "; "
            << summation.index.to_string() << "++) {"
            << "\n\t\tsum += " << summation.summand->generate(system) << ";"
            << "\n\t}"
            << "\n\t return sum;"
            << "\n}";
        system.bound_parameters.erase(summation.index.name);
    }

    return str.str();
}

std::string generate_initial_state_setter(SystemDeclarations &system)
{
    auto &initial_states = system.initial_states;

    std::stringstream str;

    str << "\n\nvoid get_initial_state(N_Vector state) {"
        << "\n    double* values = N_VGetArrayPointer(state);\n";
    for (size_t i = 0; i < initial_states.size(); ++i)
    {
        if (initial_states[i].symbol.is_list())
        {
            if (initial_states[i].symbol.parameters[0].type == ParameterType::EXPRESSION)
                continue;

            str << generate_setter_list(system, initial_states[i]);
        }
        else
        {
            if (!initial_states[i].rhs)
            {
                std::cerr << "Error: Initial state of " << initial_states[i].symbol.name << " is missing definition.\n";
                continue;
            }

            system.bound_parameters.clear();
            str << "    values[INDEX_" << initial_states[i].symbol.to_string() << "] = " << initial_states[i].rhs->generate(system) << ";\n";
        }
    }

    // Second pass to define manual overrides to the list
    for (size_t i = 0; i < initial_states.size(); ++i)
    {
        if (initial_states[i].symbol.is_list() && initial_states[i].symbol.parameters[0].type == ParameterType::EXPRESSION)
        {
            system.bound_parameters.clear();
            str << "    values[INDEX_" << initial_states[i].symbol.to_string() << "_START + (size_t)"
                << "(" << initial_states[i].symbol.parameters[0].expression->generate(system) << " - 1)" << "] = "
                << initial_states[i].rhs->generate(system) << ";\n";
        }
    }

    str << "}\n\n";

    return str.str();
}

std::string generate_derivative_definitions(SystemDeclarations &system)
{
    auto &deps = system.state_variables;

    std::stringstream str;

    for (size_t i = 0; i < deps.size(); ++i)
    {
        if (deps[i].symbol.is_list())
        {
            if (deps[i].symbol.parameters[0].type == ParameterType::EXPRESSION)
                continue;

            str << generate_derivative_list(system, deps[i]);
        }
        else
        {
            system.bound_parameters.clear();
            str << "    derivatives[INDEX_" << deps[i].symbol.to_string() << "] = " << deps[i].rhs->generate(system) << ";\n";
        }
    }
    str << "\n";

    // Second pass to define manual overrides to the list
    for (size_t i = 0; i < deps.size(); ++i)
    {
        if (deps[i].symbol.is_list() && deps[i].symbol.parameters[0].type == ParameterType::EXPRESSION)
        {
            system.bound_parameters.clear();
            str << "    derivatives[INDEX_" << deps[i].symbol.to_string() << "_START + (size_t)(" << deps[i].symbol.parameters[0].expression->generate(system)
                << " - 1)] = " << deps[i].rhs->generate(system) << ";\n";
        }
    }

    return str.str();
}

std::string generate_csv_getters(SystemDeclarations &system)
{
    std::stringstream str;

    auto &deps = system.state_variables;

    str << "std::string get_state_csv_label() {";

    str << "\n\tstd::stringstream str; "
        << "\n\tstr << \"t (seconds)\";";
    for (size_t i = 0; i < deps.size(); ++i)
    {
        if (deps[i].symbol.is_list())
        {
            str << generate_csv_list(system, deps[i].symbol);
        }
        else
        {
            str << "\n\tstr << \", " << deps[i].symbol.to_string() << "\";";
        }
    }
    for (auto out : system.additional_outputs)
    {
        str << "\n\tstr << \", " << out.label.to_string() << "\";";
    }
    str << "\n\treturn str.str();";

    str << "\n}\n\n";

    str << "std::string get_csv_line(N_Vector state) {"
        << "\n\tstd::stringstream str;"
        << "\n\tdouble* values = N_VGetArrayPointer(state);"
        << "\n\tfor (size_t i = 0; i < STATE_SIZE; ++i) {"
        << "\n\t\tstr << \", \" << values[i];"
        << "\n\t}";
        for (auto out : system.additional_outputs)
        {
            str << "\n\tstr << \", \" << " << out.rhs->generate(system) << ";";
        }
        str << "\n\treturn str.str();"
        << "\n}\n\n";

    return str.str();
}

std::string generate_system(SystemDeclarations &system)
{
    std::stringstream str;

    auto &deps = system.state_variables;

    str << "int system(sunrealtype t, N_Vector y, N_Vector ydot, void *user_data) {\n"
        << "    double* values = N_VGetArrayPointer(y);\n"
        << "    double* derivatives = N_VGetArrayPointer(ydot);\n"
        << generate_derivative_definitions(system)
        << "    return 0;\n"
        << "}";

    return str.str();
}