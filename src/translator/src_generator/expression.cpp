#include "expression.h"
#include "parse.h"


std::string FunctionDefinition::get_parameter_constraints(SystemDeclarations& system, FunctionDefinition& catchall)
{
    if (is_catchall())
    {
        std::cerr << "Warning: Tried to pull parameter constraints from a catchall definition.\n";
        return "";
    }

    if (catchall.parameters.size() != parameters.size())
    {
        std::cerr << "Warning: Catchall must have the same number of parameters as constrained definitions.\n";
        return "";
    }

    std::stringstream str;
    bool include_and = false;
    for (size_t i = 0; i < catchall.parameters.size(); ++i)
    {
        if (parameters[i].type == ParameterType::EXPRESSION)
        {
            auto& catchall_param = catchall.parameters[i];
            str << (include_and ? " && " : "") << catchall_param.symbol.value_or("ERROR") << " == (" << parameters[i].expression->generate(system) << ")";
            include_and = true;
        }
    }
    return str.str();
}

bool Function::is_state_dependent(SystemDeclarations& system)
{
    for (auto definition : definitions)
    {
        if (definition.expression->has_state_dependencies(system))
        {
            return true;
        }
    }
    return false;
}

std::string generate_parameter_value(SystemDeclarations& system, Parameter& parameter)
{
    std::stringstream str;
    switch(parameter.type)
    {
        case ParameterType::VARIABLE:
            {
            if (system.bound_parameters.count(parameter.symbol.value()))
            {
                str << "(" << parameter.symbol.value() << ")";
            }
            else
            {
                std::cerr << "Error: Tried to use unbound index variable " << parameter.symbol.value_or("ERROR") << " in definition.\n";
                return "";
            }
            }
        break;
        case ParameterType::EXPRESSION:
            {
            str << "(" << parameter.expression->generate(system) << ")";
            }
        break;
        default:
        break;
    }

    return str.str();
}

std::string SymbolExpression::generate(SystemDeclarations& system)
{
    std::stringstream str;

    SymbolType type = system.resolve_symbol_type(symbol);
    switch (type) 
    {
        case SymbolType::STATE:
            if (symbol.parameters.size() > 0) {
                auto& parameter = symbol.parameters[0];
                str << "values[INDEX_" << symbol.to_string() << "_START + " << generate_parameter_value(system, parameter) << " - 1]";
                return str.str();
            }

            str << "values[INDEX_" << symbol.to_string() << "]";
            return str.str();
        case SymbolType::PARAMETER:
            if (!system.bound_parameters.count(symbol.to_string()))
            {
                std::cerr << "Error: Used an unbound parameter." << std::endl;
                return "0";
            }
            return symbol.to_string();
        case SymbolType::FUNCTION:
            for (auto& f : system.function_definitions)
            {
                if (f.symbol != symbol) continue;
                
                if (f.is_constant(system))
                {
                    return symbol.to_string();
                }

                str << symbol.to_string() << "(";
                for (auto i = 0; i < symbol.parameters.size(); ++i)
                {
                    str << (i != 0 ? ", " : "") << generate_parameter_value(system, symbol.parameters[i]);
                }

                if (f.is_state_dependent(system))
                {
                    str << (symbol.parameters.size() == 0 ? "" : ", ") << "values";
                }

                str << ")";
                return str.str();
            }
            std::cerr << "Error: Function " << symbol.name << " is undefined.\n";
            return "0";
        case SymbolType::SUMMATION:
            for (auto& summation : system.summation_definitions)
            {
                if (summation.symbol != symbol) continue;
                return symbol.to_string() + "(values)";
            }

            std::cerr << "Error: Could not find summation definition.\n";
            return "0";
    }

    std::cerr << "Use of undefined symbol " << symbol.to_string() << ". Make sure this is intentional.\n";
    return symbol.to_string();
}

bool SymbolExpression::has_state_dependencies(SystemDeclarations& system)
{
    // Probably better as a map, having this ordered is nice for generation though
    for (StateVariable& dep : system.state_variables)
    {
        if (dep.symbol == symbol) return true;
    }

    if (symbol.type == SymbolType::SUMMATION)
    {
        for (auto& summation : system.summation_definitions)
        {
            if (summation.symbol == symbol && summation.summand->has_state_dependencies(system)) return true;
        }
    }

    return false;
}

std::string AddExpression::generate(SystemDeclarations& system)
{
    std::stringstream code;
    code << "((" << lhs->generate(system) << ") + (" << rhs->generate(system) << "))";
    return code.str();
}

std::string SubtractExpression::generate(SystemDeclarations& system)
{
    std::stringstream code;
    code << "((" << lhs->generate(system) << ") - (" << rhs->generate(system) << "))";
    return code.str();
}

std::string MultiplyExpression::generate(SystemDeclarations& system)
{
    std::stringstream code;
    code << "((" << lhs->generate(system) << ") * (" << rhs->generate(system) << "))";
    return code.str();
}

std::string DivideExpression::generate(SystemDeclarations& system)
{
    std::stringstream code;
    code << "((" << lhs->generate(system) << ") / (" << rhs->generate(system) << "))";
    return code.str();
}

std::string ExponentExpression::generate(SystemDeclarations& system)
{
    if (!base) {
        std::cerr << "Error: Exponent expression is missing base." << std::endl;
        return "";
    }

    if (!exp) {
        std::cerr << "Error: Exponent expression is missing exponent." << std::endl;
        return "";
    }

    std::stringstream code;
    code << "std::pow(" << base->generate(system) << ", " << exp->generate(system) << ")";
    return code.str();
}

std::string SqrtExpression::generate(SystemDeclarations& system)
{
    std::stringstream code;
    code << "std::sqrt(" << base->generate(system) << ")";
    return code.str();
}

std::string ExpExpression::generate(SystemDeclarations& system)
{
    std::stringstream code;
    code << "std::exp(" << exp->generate(system) << ")";
    return code.str();
}