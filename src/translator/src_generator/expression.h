#pragma once

#include <memory>
#include <sstream>
#include <vector>

#include "tokenize.h"

class SystemDeclarations;

struct FunctionDefinition
{
    std::vector<Parameter> parameters;
    std::shared_ptr<Expression> expression;

    bool is_catchall()
    {
        for (auto p : parameters) 
        {
            if (p.type != ParameterType::VARIABLE) 
                return false;
        }
        return true;
    }

    std::string get_parameter_constraints(SystemDeclarations& system, FunctionDefinition& catchall);
};

struct Function
{
    Symbol symbol;
    std::vector<FunctionDefinition> definitions;

    Function()
        : symbol("error")
    {
    }

    Function(Symbol symbol)
        : symbol(symbol)
    {

    }

    bool is_constant(SystemDeclarations& system)
    {
        return symbol.parameters.size() == 0 && !is_state_dependent(system);
    }

    bool is_state_dependent(SystemDeclarations& system);

    FunctionDefinition get_catchall_definition()
    {
        for (auto definition : definitions)
        {
            if (definition.is_catchall())
            {
                return definition;
            }
        }

        std::cerr << "Function " << symbol.to_string() << " is missing a catch-all definition.\n";
        return definitions[0];
    }
};

struct Range
{
    std::shared_ptr<Expression> start;
    std::shared_ptr<Expression> end;

    Range() {}
    Range(std::shared_ptr<Expression> start, std::shared_ptr<Expression> end)
        : start(start), end(end)
    {}
};

class Expression
{
public:
    virtual std::string generate(SystemDeclarations& system) = 0;
    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return false;
    }
};

class ConstantExpression : public Expression
{
public:
    float value;

    ConstantExpression(float value)
        : value(value)
    {}

    virtual std::string generate(SystemDeclarations& system)
    {
        std::stringstream str;
        str << value;
        return str.str();
    }
};

class SymbolExpression : public Expression
{
public:
    Symbol symbol;

    SymbolExpression(Symbol symbol)
        : symbol(symbol)
    {}

    virtual std::string generate(SystemDeclarations& system);
    virtual bool has_state_dependencies(SystemDeclarations& system);
};

class NegateExpression : public Expression
{
public:
    std::shared_ptr<Expression> negated_expression;

    NegateExpression(std::shared_ptr<Expression> negated_expression)
        : negated_expression(negated_expression)
    {}

    virtual std::string generate(SystemDeclarations& system)
    {
        std::stringstream code;
        code << "-(" << negated_expression->generate(system) << ")";
        return code.str();
    }

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return negated_expression->has_state_dependencies(system);
    }
};

class AddExpression : public Expression
{
public:
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;

    AddExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs)
        : lhs(lhs), rhs(rhs)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return lhs->has_state_dependencies(system) || rhs->has_state_dependencies(system);
    }
};

class SubtractExpression : public Expression
{
public:
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;

    SubtractExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs)
        : lhs(lhs), rhs(rhs)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return lhs->has_state_dependencies(system) || rhs->has_state_dependencies(system);
    }
};

class MultiplyExpression : public Expression
{
public:
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;

    MultiplyExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs)
        : lhs(lhs), rhs(rhs)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return lhs->has_state_dependencies(system) || rhs->has_state_dependencies(system);
    }
};

class DivideExpression : public Expression
{
public:
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;

    DivideExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs)
        : lhs(lhs), rhs(rhs)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return lhs->has_state_dependencies(system) || rhs->has_state_dependencies(system);
    }
};

class ExponentExpression : public Expression
{
public:
    std::shared_ptr<Expression> base;
    std::shared_ptr<Expression> exp;

    ExponentExpression(std::shared_ptr<Expression> base, std::shared_ptr<Expression> exp)
        : base(base), exp(exp)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return base->has_state_dependencies(system) || exp->has_state_dependencies(system);
    }
};

class SqrtExpression : public Expression
{
public:
    std::shared_ptr<Expression> base;

    SqrtExpression(std::shared_ptr<Expression> base)
        : base(base)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return base->has_state_dependencies(system);
    }
};

class ExpExpression : public Expression
{
public:
    std::shared_ptr<Expression> exp;

    ExpExpression(std::shared_ptr<Expression> exp)
        : exp(exp)
    {}

    virtual std::string generate(SystemDeclarations& system);

    virtual bool has_state_dependencies(SystemDeclarations& system)
    {
        return exp->has_state_dependencies(system);
    }
};

class RangeExpression : public Expression
{
public:
    Range range;

    RangeExpression(std::shared_ptr<Expression> start, std::shared_ptr<Expression> end)
        : range(start, end)
    {}

    virtual std::string generate(SystemDeclarations& system)
    {
        std::cerr << "Error: Range expression must be either standalone or used for a summation.\n";
        return "";
    }
};