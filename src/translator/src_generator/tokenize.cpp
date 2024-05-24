#include "tokenize.h"
#include "expression.h"

Parameter::~Parameter()
{

}

std::string get_token_type_string(TokenType type)
{
    switch (type) 
    {
        case TokenType::CONSTANT: return "CONSTANT";
        case TokenType::SYMBOL: return "SYMBOL";
        case TokenType::RANGE: return "RANGE";
        case TokenType::DERIVATIVE: return "DERIVATIVE";
        case TokenType::INITIAL: return "INITIAL";
        case TokenType::OUTPUT: return "OUTPUT";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::ADD: return "ADD";
        case TokenType::SUBTRACT: return "SUBTRACT";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::EXPONENT: return "EXPONENT";
        case TokenType::NEGATE: return "NEGATE";
        case TokenType::SQRT: return "SQRT";
        case TokenType::EXP: return "EXP";
        case TokenType::SUM: return "SUM";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::COMMA: return "COMMA";
        case TokenType::TAG: return "TAG";
        default: return "UNKNOWN";
    }
}

std::vector<Parameter> get_indices(std::string indexList)
{
    std::vector<Parameter> indices;
    std::smatch matches;

    while (indexList.size() > 0) {
        if (std::regex_search(indexList, matches, std::regex("^,? ?([0-9]+)")))
        {
            indices.push_back(Parameter());
            indices.back().type = ParameterType::EXPRESSION;
            indices.back().index_start = std::stoi(matches[1]);
            indices.back().index_end = indices.back().index_start;
            indexList = indexList.substr(matches[0].str().size());
            continue;
        }

        if (std::regex_search(indexList, matches, std::regex("^,? ?([A-Za-z_][A-Za-z_0-9]*)")))
        {
            indices.push_back(Parameter());
            indices.back().type = ParameterType::VARIABLE;
            indices.back().symbol = matches[1].str();
            indexList = indexList.substr(matches[0].str().size());
            continue;
        }
    }

    return indices;
}

std::vector<Token> tokenize(std::string line) 
{
    std::vector<Token> tokens;
    std::smatch matches;

    while (line.size() > 0) {
        
        if (std::regex_search(line, matches, std::regex("^#"))) {
            break;
        }
        
        if (std::regex_search(line, matches, std::regex("^d/dt"))) {
            tokens.push_back(Token { TokenType::DERIVATIVE });
            line = line.substr(matches[0].str().size());
            continue;
        }
        
        if (std::regex_search(line, matches, std::regex("^="))) {
            tokens.push_back(Token { TokenType::ASSIGN });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^,"))) {
            tokens.push_back(Token { TokenType::COMMA });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\+"))) {
            tokens.push_back(Token { TokenType::ADD });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\-"))) {
            if (tokens.size() > 0 && (tokens.back().type == TokenType::CONSTANT 
                || tokens.back().type == TokenType::SYMBOL || tokens.back().type == TokenType::RPAREN))
            {
                tokens.push_back(Token { TokenType::SUBTRACT });
            }
            else
            {
                tokens.push_back(Token { TokenType::NEGATE });
            }
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\*"))) {
            tokens.push_back(Token { TokenType::MULTIPLY });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^/"))) {
            tokens.push_back(Token { TokenType::DIVIDE });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\^"))) {
            tokens.push_back(Token { TokenType::EXPONENT });
            line = line.substr(1);
            continue;
        }
        
        if (std::regex_search(line, matches, std::regex("^OUTPUT"))) {
            tokens.push_back(Token { TokenType::OUTPUT });
            line = line.substr(6);
            continue;
        }
        
        if (std::regex_search(line, matches, std::regex("^INITIAL"))) {
            tokens.push_back(Token { TokenType::INITIAL });
            line = line.substr(7);
            continue;
        }
        
        if (std::regex_search(line, matches, std::regex("^SQRT"))) {
            tokens.push_back(Token { TokenType::SQRT });
            line = line.substr(4);
            continue;
        }
        
        if (std::regex_search(line, matches, std::regex("^EXP"))) {
            tokens.push_back(Token { TokenType::EXP });
            line = line.substr(3);
            continue;
        }
        
        if (std::regex_search(line, matches, std::regex("^SUM"))) {
            tokens.push_back(Token { TokenType::SUM });
            line = line.substr(3);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\("))) {
            tokens.push_back(Token { TokenType::LPAREN });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\)"))) {
            tokens.push_back(Token { TokenType::RPAREN });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\["))) {
            tokens.push_back(Token { TokenType::LBRACKET });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\]"))) {
            tokens.push_back(Token { TokenType::RBRACKET });
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^([A-Za-z_][A-Za-z_0-9]*)"))) {
            tokens.push_back(Token { TokenType::SYMBOL, Symbol(matches[0]) });
            line = line.substr(matches[0].str().size());
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^\\.\\."))) {
            tokens.push_back(Token { TokenType::RANGE } );
            line = line.substr(1);
            continue;
        }

        if (std::regex_search(line, matches, std::regex("^([0-9]+\\.?[0-9]*)"))) {
            tokens.push_back(Token { TokenType::CONSTANT, std::nullopt, std::atof(matches[1].str().c_str()) } );
            line = line.substr(matches[1].str().size());
            continue;
        }

        line = line.substr(1);
    }

    return tokens;
}
