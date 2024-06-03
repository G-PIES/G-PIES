#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <sstream>
#include <regex>

enum class ParameterType
{
    VARIABLE,
    EXPRESSION
};

class Expression;

struct Parameter {
    ParameterType type;
    std::optional<std::string> symbol;
    std::shared_ptr<Expression> expression = nullptr;
    size_t index_start = -1;
    size_t index_end = -2;

    ~Parameter();
};

enum class SymbolType 
{
    UNRESOLVED,
    STATE,
    FUNCTION,
    SUMMATION,
    PARAMETER
};

struct Symbol
{
    std::string name;
    std::vector<Parameter> parameters;
    SymbolType type = SymbolType::UNRESOLVED;

    Symbol(std::string sym)
        : name(sym)
    {
    }

    Symbol(std::string sym, std::vector<Parameter> parameters)
        : name(sym), parameters(parameters)
    {
    }

    std::string to_string()
    {
        return name;
    }

    bool operator==(const Symbol& op)
    {
        return op.name == name;
    }

    bool operator !=(const Symbol& op)
    {
        return op.name != name;
    }

    bool is_list()
    {
        return type == SymbolType::STATE && parameters.size() > 0;
    }
};

enum class TokenType {
    CONSTANT,
    SYMBOL,
    DERIVATIVE,
    RANGE,
    INITIAL,
    OUTPUT,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    EXPONENT,
    NEGATE,
    SQRT,
    EXP,
    SUM,
    ASSIGN,
    COMMA,
    TAG
};

std::string get_token_type_string(TokenType type);

struct Token {
    TokenType type;
    std::optional<Symbol> symbol;
    std::optional<float> value; // Used by CONSTANT tokens
    size_t list_size = 0; // Used by LIST tokens
    

    std::string to_string() {
        std::stringstream str;

        str << "[" << get_token_type_string(type);
        if (symbol.has_value()) {
            str << ", " << symbol.value().to_string();
        }
        if (value.has_value()) {
            str << ", " << value.value();
        }
        str << "]";

        return str.str();
    }
};

std::vector<Token> tokenize(std::string line);