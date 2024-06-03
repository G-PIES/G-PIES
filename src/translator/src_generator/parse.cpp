#include "parse.h"

std::vector<Parameter> parse_parameters(SystemDeclarations &system, std::vector<Token> tokens);
std::shared_ptr<Expression> parse_expression(SystemDeclarations &system, std::vector<Token> &tokens);

unsigned int Summation::next_id = 0;

Symbol parse_symbol(SystemDeclarations &system, std::vector<Token> &tokens)
{
    Symbol symbol = tokens[0].symbol.value();

    TokenType end_token;
    switch (tokens[1].type)
    {
    case TokenType::LBRACKET:
        end_token = TokenType::RBRACKET;
        symbol.type = SymbolType::STATE;
        break;
    case TokenType::LPAREN:
        end_token = TokenType::RPAREN;
        symbol.type = SymbolType::FUNCTION;
        break;
    default:
        tokens.erase(tokens.begin());
        symbol.type = SymbolType::FUNCTION;
        return symbol;
    }

    auto end_it = std::find_if(tokens.begin() + 1, tokens.end(), [&](Token token)
                               { return token.type == end_token; });
    if (end_it == tokens.end())
    {
        tokens.clear();
        symbol.type = SymbolType::UNRESOLVED;
        return symbol;
    }

    std::vector<Token> parameters_tokens = std::vector<Token>(tokens.begin() + 2, end_it);
    std::vector<Parameter> parameters = parse_parameters(system, parameters_tokens);

    symbol.parameters = parameters;
    tokens.erase(tokens.begin(), end_it + 1);
    return symbol;
}

enum Priority
{
    PRIORITY_ALL = 0,
    PRIORITY_ADD = 1,
    PRIORITY_MUL = 2,
    PRIORITY_EXP = 3,
    PRIORITY_PAREN = 4,
};

std::vector<Token> pop_subexpr_tokens(std::vector<Token> &tokens, Priority priority)
{
    int paren_level = 0;
    int bracket_level = 0;
    auto it = tokens.begin();
    for (; it != tokens.end(); ++it)
    {
        if (it->type == TokenType::LPAREN)
            paren_level += 1;
        if (it->type == TokenType::RPAREN)
            paren_level -= 1;
        if (paren_level > 0)
            continue;
        if (paren_level < 0)
            break;

        if (it->type == TokenType::LBRACKET)
            bracket_level += 1;
        if (it->type == TokenType::RBRACKET)
            bracket_level -= 1;
        if (bracket_level > 0)
            continue;
        if (bracket_level < 0)
            break;

        if (priority >= Priority::PRIORITY_ADD && (it->type == TokenType::ADD || it->type == TokenType::SUBTRACT))
        {
            // Special exception allowing negate expressions to be alone in exponents
            if (priority == Priority::PRIORITY_EXP && it == tokens.begin() + 1 && it->type == TokenType::SUBTRACT)
                continue;

            break;
        }

        if (priority >= Priority::PRIORITY_MUL && (it->type == TokenType::MULTIPLY || it->type == TokenType::DIVIDE))
            break;

        if (priority >= Priority::PRIORITY_EXP && it->type == TokenType::EXPONENT)
            break;
    }

    auto subexpr_tokens = std::vector<Token>(tokens.begin(), it);
    tokens.erase(tokens.begin(), it);
    return subexpr_tokens;
}

std::shared_ptr<Expression> parse_unary_expression(SystemDeclarations &system, std::vector<Token> &tokens)
{
    if (tokens[1].type == TokenType::LPAREN)
    {
        tokens.erase(tokens.begin());
        auto subexpr_tokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_PAREN);
        return parse_expression(system, subexpr_tokens);
    }

    tokens.erase(tokens.begin());
    auto subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_ADD);
    return parse_expression(system, subexprTokens);
}

std::optional<Range> parse_range(SystemDeclarations &system, std::vector<Token> tokens)
{
    auto subexpr_it = std::find_if(tokens.begin(), tokens.end(), [](Token t)
                                   { return t.type == TokenType::RANGE; });
    if (subexpr_it == tokens.end())
    {
        std::cerr << "Error: Missing range in expression.\n";
        return std::nullopt;
    }
    auto range_start_tokens = std::vector<Token>(tokens.begin(), subexpr_it);
    auto range_start_expression = parse_expression(system, range_start_tokens);
    if (!range_start_expression)
    {
        std::cerr << "Error: Failed to parse range start.\n";
        return std::nullopt;
    }

    tokens.erase(tokens.begin(), subexpr_it + 1);

    auto range_end_tokens = std::vector<Token>(tokens.begin(), tokens.end());
    auto range_end_expression = parse_expression(system, range_end_tokens);
    if (!range_end_expression)
    {
        std::cerr << "Error: Failed to parse range end.\n";
        return std::nullopt;
    }
    tokens.erase(tokens.begin(), subexpr_it);

    return Range{range_start_expression, range_end_expression};
}

std::shared_ptr<Expression> parse_sum(SystemDeclarations &system, std::vector<Token> &tokens)
{   
    tokens.erase(tokens.begin());
    if (tokens[0].type != TokenType::LPAREN)
    {
        std::cerr << "Error: SUM expression must be followed by a left parenthesis.\n";
        return nullptr;
    }
    tokens.erase(tokens.begin());

    if (tokens[0].type != TokenType::SYMBOL || !tokens[0].symbol.has_value())
    {
        std::cerr << "Error: SUM expression is missing a name for its range.\n";
        return nullptr;
    }
    auto index_symbol = tokens[0].symbol;
    tokens.erase(tokens.begin());

    if (tokens[0].type != TokenType::ASSIGN)
    {
        std::cerr << "Error: SUM expression is missing assignment of its range.\n";
        return nullptr;
    }
    tokens.erase(tokens.begin());

    auto subexpr_it = std::find_if(tokens.begin(), tokens.end(), [](Token t)
                              { return t.type == TokenType::COMMA; });
    if (subexpr_it == tokens.end())
    {
        std::cerr << "Error: Missing comma in SUM expression.\n";
        return nullptr;
    }

    auto range = parse_range(system, std::vector(tokens.begin(), subexpr_it));
    if (!range.has_value())
    {
        std::cerr << "Error: Failed to parse summation range.\n";
        return nullptr;
    }
    tokens.erase(tokens.begin(), subexpr_it + 1);

    auto summand_tokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_ALL);
    tokens.erase(tokens.begin());

    auto summand_expression = parse_expression(system, summand_tokens);
    if (!summand_expression)
    {
        std::cerr << "Error: Failed to parse SUM body.\n";
        return nullptr;
    }

    Symbol summation_symbol("__summation_" + std::to_string(Summation::next_id++));
    summation_symbol.type = SymbolType::SUMMATION;
    system.summation_definitions.push_back(
        Summation{summation_symbol, index_symbol.value(), summand_expression, range.value()});

    return std::make_shared<SymbolExpression>(summation_symbol);
}

std::shared_ptr<Expression> parse_expression(SystemDeclarations &system, std::vector<Token> &tokens)
{
    std::shared_ptr<Expression> expression = nullptr;
    std::vector<Token> subexprTokens;

    while (tokens.size() > 0)
    {
        switch (tokens[0].type)
        {
        case TokenType::LPAREN:
        {
            tokens.erase(tokens.begin());
            auto subexpr_tokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_ALL);
            tokens.erase(tokens.begin());
            expression = parse_expression(system, subexpr_tokens);
        }
            continue;
        case TokenType::SYMBOL:
            expression = std::make_shared<SymbolExpression>(parse_symbol(system, tokens));
            continue;
        case TokenType::CONSTANT:
            expression = std::make_shared<ConstantExpression>(tokens[0].value.value());
            tokens.erase(tokens.begin());
            continue;
        case TokenType::RANGE:
            tokens.erase(tokens.begin());
            subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_EXP);
            expression = std::make_shared<RangeExpression>(expression, parse_expression(system, subexprTokens));
            continue;
        case TokenType::ADD:
            tokens.erase(tokens.begin());
            subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_ADD);
            expression = std::make_shared<AddExpression>(expression, parse_expression(system, subexprTokens));
            continue;
        case TokenType::SUBTRACT:
            tokens.erase(tokens.begin());
            subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_ADD);
            expression = std::make_shared<SubtractExpression>(expression, parse_expression(system, subexprTokens));
            continue;
        case TokenType::MULTIPLY:
            tokens.erase(tokens.begin());
            subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_MUL);
            expression = std::make_shared<MultiplyExpression>(expression, parse_expression(system, subexprTokens));
            continue;
        case TokenType::DIVIDE:
            tokens.erase(tokens.begin());
            subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_MUL);
            expression = std::make_shared<DivideExpression>(expression, parse_expression(system, subexprTokens));
            continue;
        case TokenType::EXPONENT:
            tokens.erase(tokens.begin());
            subexprTokens = pop_subexpr_tokens(tokens, Priority::PRIORITY_EXP);
            expression = std::make_unique<ExponentExpression>(expression, parse_expression(system, subexprTokens));
            continue;
        case TokenType::NEGATE:
        {
            auto unary_expr = parse_unary_expression(system, tokens);
            if (!unary_expr)
            {
                std::cerr << "Error: Negate doesn't have a valid expression.\n";
            }
            expression = std::make_shared<NegateExpression>(unary_expr);
        }
            continue;
        case TokenType::SQRT:
        {
            auto unary_expr = parse_unary_expression(system, tokens);
            if (!unary_expr)
            {
                std::cerr << "Error: Sqrt doesn't have a valid expression.\n";
            }
            expression = std::make_shared<SqrtExpression>(unary_expr);
        }
            continue;
        case TokenType::EXP:
        {
            auto unary_expr = parse_unary_expression(system, tokens);
            if (!unary_expr)
            {
                std::cerr << "Error: Sqrt doesn't have a valid expression.\n";
            }
            expression = std::make_shared<ExpExpression>(unary_expr);
        }
            continue;
        case TokenType::SUM:
            expression = parse_sum(system, tokens);
            continue;
        default:
            std::cerr << "Unable to parse expression starting with token " << tokens[0].to_string() << "\n";
            return nullptr;
        }
    }

    return expression;
}

std::vector<Parameter> parse_parameters(SystemDeclarations &system, std::vector<Token> tokens)
{
    std::vector<Parameter> parameters;

    while (1)
    {
        auto it = std::find_if(tokens.begin(), tokens.end(), [](Token token)
                               { return token.type == TokenType::COMMA; });

        std::vector<Token> parameter_tokens = std::vector<Token>(tokens.begin(), it);

        Parameter parameter;
        if (parameter_tokens.size() == 1 && parameter_tokens[0].type == TokenType::SYMBOL)
        {
            parameter.type = ParameterType::VARIABLE;
            parameter.symbol = parameter_tokens[0].symbol.value().name;
            for (auto d : system.function_definitions)
            {
                if (d.symbol.name == parameter_tokens[0].symbol.value().name)
                {
                    parameter.type = ParameterType::EXPRESSION;
                    parameter.expression = std::make_shared<SymbolExpression>(parameter.symbol.value());
                }
            }
            parameters.push_back(parameter);
        }
        else
        {
            parameter.type = ParameterType::EXPRESSION;
            parameter.expression = parse_expression(system, parameter_tokens);
            parameters.push_back(parameter);
        }

        if (it == tokens.end())
            break;

        tokens.erase(tokens.begin(), it + 1);
    }

    return parameters;
}

void parse_state_definition(SystemDeclarations &system, std::vector<Token> tokens)
{
    if (tokens.size() < 2 || !tokens[1].symbol.has_value())
    {
        std::cerr << "Error: Derivative is missing symbol.\n";
        return;
    }

    tokens.erase(tokens.begin()); // Remove the derivative token
    Symbol symbol = parse_symbol(system, tokens);
    tokens.erase(tokens.begin()); // Remove the = token

    std::shared_ptr<Expression> rhs = parse_expression(system, tokens);
    if (!rhs)
    {
        std::cerr << "Error: Malformed rhs expression for derivative of " << symbol.name << "\n";
        return;
    }

    system.state_variables.push_back(StateVariable(symbol, std::move(rhs)));
}

void parse_initial_value(SystemDeclarations &system, std::vector<Token> tokens)
{
    tokens.erase(tokens.begin());                 // Remove "INITIAL"
    Symbol symbol = parse_symbol(system, tokens); // Grab and remove the symbol
    tokens.erase(tokens.begin());                 // Remove "="

    std::shared_ptr<Expression> expression = parse_expression(system, tokens);
    if (!expression)
    {
        std::cerr << "Error: Malformed expression for initial value of " << symbol.name << "\n";
        return;
    }
    system.initial_states.push_back(InitialState{symbol, expression});
}

void parse_symbol_declaration(SystemDeclarations &system, std::vector<Token> tokens)
{
    Symbol symbol = parse_symbol(system, tokens);

    tokens.erase(tokens.begin()); // Remove '='
    std::shared_ptr<Expression> expression = parse_expression(system, tokens);
    if (!expression)
    {
        std::cerr << "Malformed expression: " << symbol.name << "\n";
        return;
    }

    if (auto range_expr = dynamic_cast<RangeExpression*>(expression.get()))
    {
        system.ranges[symbol.name] = range_expr->range; 
        return;
    }

    if (symbol.type == SymbolType::FUNCTION)
    {
        auto function = system.find_function_definition(symbol);
        if (function == nullptr)
        {
            system.function_definitions.push_back(Function(symbol));
            function = system.find_function_definition(symbol);
        }

        function->definitions.push_back(FunctionDefinition{symbol.parameters, expression});
        return;
    }
}

void parse_output_value(SystemDeclarations &system, std::vector<Token> tokens)
{
    tokens.erase(tokens.begin());                 // Remove "OUTPUT"
    Symbol symbol = parse_symbol(system, tokens); // Grab and remove the label symbol

    std::shared_ptr<Expression> expression = parse_expression(system, tokens);
    if (!expression)
    {
        std::cerr << "Error: Malformed expression for output value " << symbol.name << "\n";
        return;
    }
    system.additional_outputs.push_back(ExpressionOutput{symbol, expression});
}

void parse_declaration(SystemDeclarations &system, std::string line)
{
    std::vector<Token> tokens = tokenize(line);

    if (tokens.size() == 0)
        return;

    switch (tokens[0].type)
    {
    case TokenType::DERIVATIVE:
        parse_state_definition(system, tokens);
        break;
    case TokenType::SYMBOL:
        parse_symbol_declaration(system, tokens);
        break;
    case TokenType::INITIAL:
        parse_initial_value(system, tokens);
        break;
    case TokenType::OUTPUT:
        parse_output_value(system, tokens);
        break;
    }
}

std::vector<std::string> collect_lines(std::ifstream& stream)
{
    std::vector<std::string> lines;
    std::string line;

    std::string chunk;
    while (getline(stream, chunk))
    {
        if (std::regex_search(chunk, std::regex("^[ \\t]")))
        {
            line += chunk;
            continue;
        }

        std::smatch matches;
        if (std::regex_search(chunk, matches, std::regex("(.*)#.*")))
        {
            lines.push_back(line);
            line = "";
            line += matches[1];
            continue;
        }

        lines.push_back(line);
        line = "";
        line += chunk;
    }

    lines.push_back(line);
    line.clear();

    return lines;
}

void read_system(SystemDeclarations &system, std::ifstream &stream)
{
    auto lines = collect_lines(stream);

    for (auto line : lines) {
        parse_declaration(system, line);
    }
}