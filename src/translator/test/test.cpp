#include <string>

#include <gtest/gtest.h>

#include "../src_generator/tokenize.h"
#include "../src_generator/parse.h"


TEST(Tokenize, DerivativeTokens) 
{
    std::string text = "d/dt C";
    std::vector<Token> tokens = tokenize(text);

    EXPECT_EQ(tokens.size(), 2);

    EXPECT_EQ(tokens[0].type, TokenType::DERIVATIVE);
    
    EXPECT_EQ(tokens[1].type, TokenType::SYMBOL);
    EXPECT_TRUE(tokens[1].symbol.has_value());
    EXPECT_FALSE(tokens[1].value.has_value());
    EXPECT_EQ(tokens[1].symbol.value().name, std::string("C"));
}

TEST(Tokenize, IndexedDerivativeTokens) 
{
    std::string text = "d/dt C[n, x,z]";
    std::vector<Token> tokens = tokenize(text);

    EXPECT_EQ(tokens.size(), 9);
    EXPECT_EQ(tokens[0].type, TokenType::DERIVATIVE);
    EXPECT_EQ(tokens[1].type, TokenType::SYMBOL);
    EXPECT_EQ(tokens[2].type, TokenType::LBRACKET);
    EXPECT_EQ(tokens[3].type, TokenType::SYMBOL);
    EXPECT_EQ(tokens[4].type, TokenType::COMMA);
    EXPECT_EQ(tokens[5].type, TokenType::SYMBOL);
    EXPECT_EQ(tokens[6].type, TokenType::COMMA);
    EXPECT_EQ(tokens[7].type, TokenType::SYMBOL);
    EXPECT_EQ(tokens[8].type, TokenType::RBRACKET);

    EXPECT_EQ(tokens[1].symbol.value().name, "C");
    EXPECT_EQ(tokens[3].symbol.value().name, "n");
    EXPECT_EQ(tokens[5].symbol.value().name, "x");
    EXPECT_EQ(tokens[7].symbol.value().name, "z");
}

TEST(Tokenize, RangeToken) 
{
    std::string text = "1 .. 3";
    std::vector<Token> tokens = tokenize(text);

    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, TokenType::CONSTANT);
    EXPECT_EQ(tokens[1].type, TokenType::RANGE);
    EXPECT_EQ(tokens[2].type, TokenType::CONSTANT);
}

TEST(Tokenize, SqrtToken) 
{
    std::string text = "SQRT(q)";
    std::vector<Token> tokens = tokenize(text);
    
    EXPECT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::SQRT);
}

TEST(Parse, parse_state_definition)
{
    std::vector<Token> tokens = tokenize("d/dt C = C");

    SystemDeclarations system;
    parse_state_definition(system, tokens);

    EXPECT_EQ(system.state_variables.size(), 1);

    auto& var = system.state_variables[0];

    EXPECT_EQ(system.state_variables[0].symbol.name, std::string("C"));
    EXPECT_EQ(typeid(*var.rhs.get()), typeid(SymbolExpression));
}

TEST(Parse, AddExpression)
{
    std::vector<Token> tokens = tokenize("A + B");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(AddExpression));

    AddExpression& addexpr = *dynamic_cast<AddExpression*>(expr.get());
    EXPECT_EQ(typeid(*addexpr.lhs.get()), typeid(SymbolExpression));
    EXPECT_EQ(typeid(*addexpr.rhs.get()), typeid(SymbolExpression));

    SymbolExpression& lhsExpr = *dynamic_cast<SymbolExpression*>(addexpr.lhs.get());
    SymbolExpression& rhsExpr = *dynamic_cast<SymbolExpression*>(addexpr.rhs.get());
    EXPECT_EQ(lhsExpr.symbol.name, std::string("A"));
    EXPECT_EQ(rhsExpr.symbol.name, std::string("B"));
}

TEST(Parse, SubtractExpression)
{
    std::vector<Token> tokens = tokenize("A - B");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(SubtractExpression));

    SubtractExpression& subexpr = *dynamic_cast<SubtractExpression*>(expr.get());
    EXPECT_EQ(typeid(*subexpr.lhs.get()), typeid(SymbolExpression));
    EXPECT_EQ(typeid(*subexpr.rhs.get()), typeid(SymbolExpression));

    SymbolExpression& lhsExpr = *dynamic_cast<SymbolExpression*>(subexpr.lhs.get());
    SymbolExpression& rhsExpr = *dynamic_cast<SymbolExpression*>(subexpr.rhs.get());
    EXPECT_EQ(lhsExpr.symbol.name, std::string("A"));
    EXPECT_EQ(rhsExpr.symbol.name, std::string("B"));
}

TEST(Parse, NegateParentheses)
{
    std::vector<Token> tokens = tokenize("-(A)");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(NegateExpression));

    NegateExpression& negate_expr = *dynamic_cast<NegateExpression*>(expr.get());
    EXPECT_EQ(typeid(*negate_expr.negated_expression.get()), typeid(SymbolExpression));
    SymbolExpression& negatedExpression = *dynamic_cast<SymbolExpression*>(negate_expr.negated_expression.get());
    EXPECT_EQ(negatedExpression.symbol.name, std::string("A"));
}

TEST(Parse, TrickyNegate)
{
    std::vector<Token> tokens = tokenize("- A + B");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(AddExpression));

    AddExpression& addexpr = *dynamic_cast<AddExpression*>(expr.get());
    EXPECT_EQ(typeid(*addexpr.lhs.get()), typeid(NegateExpression));
    EXPECT_EQ(typeid(*addexpr.rhs.get()), typeid(SymbolExpression));

    NegateExpression& lhsExpr = *dynamic_cast<NegateExpression*>(addexpr.lhs.get());
    EXPECT_EQ(typeid(*lhsExpr.negated_expression.get()), typeid(SymbolExpression));
    SymbolExpression& negatedExpression = *dynamic_cast<SymbolExpression*>(lhsExpr.negated_expression.get());
    EXPECT_EQ(negatedExpression.symbol.name, std::string("A"));

    SymbolExpression& rhsExpr = *dynamic_cast<SymbolExpression*>(addexpr.rhs.get());
    EXPECT_EQ(rhsExpr.symbol.name, std::string("B"));
}

TEST(Parse, TrickyExponent)
{
    std::vector<Token> tokens = tokenize("- A ^ C[1] + B");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(AddExpression));

    AddExpression& addexpr = *dynamic_cast<AddExpression*>(expr.get());
    EXPECT_EQ(typeid(*addexpr.lhs.get()), typeid(NegateExpression));
    EXPECT_EQ(typeid(*addexpr.rhs.get()), typeid(SymbolExpression));

    NegateExpression& lhsExpr = *dynamic_cast<NegateExpression*>(addexpr.lhs.get());
    EXPECT_EQ(typeid(*lhsExpr.negated_expression.get()), typeid(ExponentExpression));

    ExponentExpression& exponent_expression = *dynamic_cast<ExponentExpression*>(lhsExpr.negated_expression.get());
    EXPECT_EQ(typeid(*exponent_expression.exp.get()), typeid(SymbolExpression));

    SymbolExpression& exp = *dynamic_cast<SymbolExpression*>(exponent_expression.exp.get());
    EXPECT_EQ(exp.symbol.parameters.size(), 1);

    SymbolExpression& rhsExpr = *dynamic_cast<SymbolExpression*>(addexpr.rhs.get());
    EXPECT_EQ(rhsExpr.symbol.name, std::string("B"));
}

TEST(Parse, OrderOfOperations)
{
    std::vector<Token> tokens = tokenize("A + B * C ^ D * E + F");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(AddExpression));
    AddExpression add_expr_1 = *dynamic_cast<AddExpression*>(expr.get());

    EXPECT_EQ(typeid(*add_expr_1.lhs.get()), typeid(AddExpression));
    AddExpression add_expr_2 = *dynamic_cast<AddExpression*>(add_expr_1.lhs.get());

    EXPECT_EQ(typeid(*add_expr_2.rhs.get()), typeid(MultiplyExpression));
    MultiplyExpression multiply_expr_1 = *dynamic_cast<MultiplyExpression*>(add_expr_2.rhs.get());

    EXPECT_EQ(typeid(*multiply_expr_1.lhs.get()), typeid(MultiplyExpression));
    MultiplyExpression multiply_expr_2 = *dynamic_cast<MultiplyExpression*>(multiply_expr_1.lhs.get());

    EXPECT_EQ(typeid(*multiply_expr_2.rhs.get()), typeid(ExponentExpression));
    ExponentExpression exponent_expr = *dynamic_cast<ExponentExpression*>(multiply_expr_2.rhs.get());
}

TEST(Parse, Sqrt)
{
    std::vector<Token> tokens = tokenize("SQRT(7)");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(SqrtExpression));

    SqrtExpression& sqrtexpr = *dynamic_cast<SqrtExpression*>(expr.get());
    EXPECT_EQ(typeid(*sqrtexpr.base.get()), typeid(ConstantExpression));

    ConstantExpression& constant_expression = *dynamic_cast<ConstantExpression*>(sqrtexpr.base.get());
    EXPECT_EQ(constant_expression.value, 7);
}

TEST(Parse, Exp)
{
    std::vector<Token> tokens = tokenize("EXP(7 + 8) / 10");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(DivideExpression));
    DivideExpression& divexpr = *dynamic_cast<DivideExpression*>(expr.get());

    EXPECT_EQ(typeid(*divexpr.lhs.get()), typeid(ExpExpression));
    ExpExpression& expexpr = *dynamic_cast<ExpExpression*>(divexpr.lhs.get());
    EXPECT_EQ(typeid(*expexpr.exp.get()), typeid(AddExpression));
}

TEST(Parse, ScientificNotation)
{
    std::vector<Token> tokens = tokenize("10 ^ -3");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(ExponentExpression));

    ExponentExpression& exp_expr = *dynamic_cast<ExponentExpression*>(expr.get());
    EXPECT_EQ(typeid(*exp_expr.base.get()), typeid(ConstantExpression));
    EXPECT_EQ(typeid(*exp_expr.exp.get()), typeid(NegateExpression));

    ConstantExpression& base_expression = *dynamic_cast<ConstantExpression*>(exp_expr.base.get());
    EXPECT_EQ(base_expression.value, 10);

    NegateExpression& exp_expression = *dynamic_cast<NegateExpression*>(exp_expr.exp.get());

    ConstantExpression& negated_expression = *dynamic_cast<ConstantExpression*>(exp_expression.negated_expression.get());
    EXPECT_EQ(negated_expression.value, 3);
}

TEST(Parse, NegateAndDivide)
{
    std::vector<Token> tokens = tokenize("- A / B");
    SystemDeclarations system;
    std::shared_ptr<Expression> expr = parse_expression(system, tokens);

    EXPECT_EQ(typeid(*expr.get()), typeid(NegateExpression));

    NegateExpression& negatedexpr = *dynamic_cast<NegateExpression*>(expr.get());

    DivideExpression& divexpr = *dynamic_cast<DivideExpression*>(negatedexpr.negated_expression.get());
    EXPECT_EQ(typeid(*divexpr.lhs.get()), typeid(SymbolExpression));
    EXPECT_EQ(typeid(*divexpr.rhs.get()), typeid(SymbolExpression));

    SymbolExpression& lhsExpr = *dynamic_cast<SymbolExpression*>(divexpr.lhs.get());
    EXPECT_EQ(lhsExpr.symbol.name, std::string("A"));

    SymbolExpression& rhsExpr = *dynamic_cast<SymbolExpression*>(divexpr.rhs.get());
    EXPECT_EQ(rhsExpr.symbol.name, std::string("B"));
}

TEST(Parse, IndexedSymbol)
{
    std::vector<Token> tokens = tokenize("INITIAL G[1, n, q+3] = C");

    SystemDeclarations system;
    parse_initial_value(system, tokens);

    EXPECT_EQ(system.initial_states.size(), 1);
    EXPECT_EQ(system.initial_states[0].symbol.parameters.size(), 3);

    auto& indices = system.initial_states[0].symbol.parameters;
    EXPECT_EQ(indices[0].type, ParameterType::EXPRESSION);
    EXPECT_EQ(indices[1].type, ParameterType::VARIABLE);
    EXPECT_EQ(indices[2].type, ParameterType::EXPRESSION);
    EXPECT_TRUE(dynamic_cast<AddExpression*>(indices[2].expression.get()));
}

TEST(Parse, IndexedExpression)
{
    std::vector<Token> tokens = tokenize("d/dt G = -C[1, n, g - 1]");

    SystemDeclarations system;
    parse_state_definition(system, tokens);

    EXPECT_EQ(system.state_variables.size(), 1);

    NegateExpression* negated_expr = dynamic_cast<NegateExpression*>(system.state_variables[0].rhs.get());
    EXPECT_TRUE(negated_expr);
    SymbolExpression* sym_expr = dynamic_cast<SymbolExpression*>(negated_expr->negated_expression.get());
    EXPECT_TRUE(sym_expr);
    EXPECT_EQ(sym_expr->symbol.parameters.size(), 3);

    auto& indices = sym_expr->symbol.parameters;
    EXPECT_EQ(indices[0].type, ParameterType::EXPRESSION);
    EXPECT_EQ(indices[1].type, ParameterType::VARIABLE);
    EXPECT_EQ(indices[2].type, ParameterType::EXPRESSION);
    EXPECT_TRUE(dynamic_cast<SubtractExpression*>(indices[2].expression.get()));
}

TEST(Parse, SummationDeclaration)
{
    std::vector<Token> tokens = tokenize("G = SUM(x = 1 .. 5, x^2)");

    SystemDeclarations system;
    parse_symbol_declaration(system, tokens);

    EXPECT_EQ(system.summation_definitions.size(), 1);
    EXPECT_EQ(system.function_definitions.size(), 1);
    EXPECT_TRUE(dynamic_cast<ExponentExpression*>(system.summation_definitions[0].summand.get()));
}

TEST(Parse, RangeDeclaration)
{
    std::vector<Token> tokens = tokenize("G = 1 .. END_OF_RANGE");

    SystemDeclarations system;
    parse_symbol_declaration(system, tokens);

    EXPECT_EQ(system.ranges.size(), 1);
    EXPECT_EQ(system.state_variables.size(), 0);
}

TEST(Parse, Range)
{
    std::vector<Token> tokens = tokenize("G = 1 .. 5");

    SystemDeclarations system;
    parse_symbol_declaration(system, tokens);
    
    EXPECT_EQ(system.ranges.size(), 1);

    auto start = dynamic_cast<ConstantExpression*>(system.ranges["G"].start.get());
    EXPECT_TRUE(start);
    EXPECT_EQ(start->value, 1);

    auto end = dynamic_cast<ConstantExpression*>(system.ranges["G"].end.get());
    EXPECT_TRUE(end);
    EXPECT_EQ(end->value, 5);
}

TEST(Parse, IndexedExpression2)
{
    std::vector<Token> tokens = tokenize("d/dt C[n] = G - C[n]");

    SystemDeclarations system;
    parse_state_definition(system, tokens);

    EXPECT_EQ(system.state_variables.size(), 1);

    SubtractExpression* sub_expr = dynamic_cast<SubtractExpression*>(system.state_variables[0].rhs.get());
    EXPECT_TRUE(sub_expr);

    SymbolExpression* sym_expr = dynamic_cast<SymbolExpression*>(sub_expr->lhs.get());
    EXPECT_TRUE(sym_expr);
    
    SymbolExpression* sym2_expr = dynamic_cast<SymbolExpression*>(sub_expr->rhs.get());
    auto& indices = sym2_expr->symbol.parameters;
    EXPECT_EQ(indices.size(), 1);
    EXPECT_EQ(indices[0].type, ParameterType::VARIABLE);
    EXPECT_TRUE(indices[0].symbol.has_value());
}

TEST(Generate, StateDefinition)
{
    std::vector<Token> tokens = tokenize("d/dt C[n] = 1.0 - C[n]");

    SystemDeclarations system;
    system.ranges["n"] = Range();
    parse_state_definition(system, tokens);
    system.bound_parameters["n"] = true;
    
    EXPECT_TRUE(system.state_variables.size() == 1);
    ASSERT_EQ(system.state_variables[0].rhs->generate(system), "((1) - (values[INDEX_C_START + (n) - 1]))");
}