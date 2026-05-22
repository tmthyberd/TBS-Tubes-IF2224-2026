#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "semantic/ASTBuilder.hpp"
#include "semantic/symbol_table/Symbol_Table.hpp"
#include "semantic/visitor/SemanticVisitor.hpp"
#include "semantic/error/SemanticError.hpp"

#include "parser/Parser.hpp"
#include "parser/core/TokenStream.hpp"
#include "parser/core/ParseTreeNode.hpp"
#include "parser/core/TreePrinter.hpp"

#include "lexer/lexer.h"
#include "lexer/token.h"


void print_usage(const char *prog_name)
{
    std::cerr << "Arion Compiler - Semantic Analysis (Milestone 3)\n"
              << "Usage:\n"
              << "  " << prog_name << " <input_source.txt>\n"
              << "  " << prog_name << " <input_source.txt> <output.txt>\n";
}

std::string read_file(const std::string &path)
{
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Cannot open file: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TokenType::PROGRAMSY:   return "programsy";
    case TokenType::VARSY:       return "varsy";
    case TokenType::CONSTSY:     return "constsy";
    case TokenType::TYPESY:      return "typesy";
    case TokenType::BEGINSY:     return "beginsy";
    case TokenType::ENDSY:       return "endsy";
    case TokenType::IFSY:        return "ifsy";
    case TokenType::THENSY:      return "thensy";
    case TokenType::ELSESY:      return "elsesy";
    case TokenType::WHILESY:     return "whilesy";
    case TokenType::DOSY:        return "dosy";
    case TokenType::FORSY:       return "forsy";
    case TokenType::TOSY:        return "tosy";
    case TokenType::DOWNTOSY:    return "downtosy";
    case TokenType::REPEATSY:    return "repeatsy";
    case TokenType::UNTILSY:     return "untilsy";
    case TokenType::CASESY:      return "casesy";
    case TokenType::OFSY:        return "ofsy";
    case TokenType::PROCEDURESY: return "proceduresy";
    case TokenType::FUNCTIONSY:  return "functionsy";
    case TokenType::ARRAYSY:     return "arraysy";
    case TokenType::RECORDSY:    return "recordsy";
    case TokenType::NOTSY:       return "notsy";
    case TokenType::ANDSY:       return "andsy";
    case TokenType::ORSY:        return "orsy";
    case TokenType::IDIV:        return "idiv";
    case TokenType::IMOD:        return "imod";
    case TokenType::IDENT:       return "ident";
    case TokenType::INTCON:      return "intcon";
    case TokenType::REALCON:     return "realcon";
    case TokenType::CHARCON:     return "charcon";
    case TokenType::STRING:      return "string";
    case TokenType::PLUS:        return "plus";
    case TokenType::MINUS:       return "minus";
    case TokenType::TIMES:       return "times";
    case TokenType::RDIV:        return "rdiv";
    case TokenType::EQL:         return "eql";
    case TokenType::NEQ:         return "neq";
    case TokenType::LSS:         return "lss";
    case TokenType::LEQ:         return "leq";
    case TokenType::GTR:         return "gtr";
    case TokenType::GEQ:         return "geq";
    case TokenType::BECOMES:     return "becomes";
    case TokenType::SEMICOLON:   return "semicolon";
    case TokenType::COLON:       return "colon";
    case TokenType::COMMA:       return "comma";
    case TokenType::PERIOD:      return "period";
    case TokenType::LPARENT:     return "lparent";
    case TokenType::RPARENT:     return "rparent";
    case TokenType::LBRACK:      return "lbrack";
    case TokenType::RBRACK:      return "rbrack";
    case TokenType::COMMENT:     return "comment";
    case TokenType::END_OF_FILE: return "eof";
    default:                     return "unknown";
    }
}

void write_tokens_to_file(const std::vector<Token> &tokens, const std::string &path, bool skip_comments = true)
{
    std::ofstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Cannot write token file: " + path);

    for (const Token &tok : tokens)
    {
        if (tok.type == TokenType::END_OF_FILE) break;
        if (skip_comments && tok.type == TokenType::COMMENT) continue;

        std::string type_str = token_type_to_string(tok.type);

        switch (tok.type)
        {
        case TokenType::IDENT:
        case TokenType::INTCON:
        case TokenType::REALCON:
        case TokenType::CHARCON:
        case TokenType::STRING:
        case TokenType::COMMENT:
            f << type_str << " (" << tok.value << ")\n";
            break;
        default:
            f << type_str << "\n";
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    std::string input_path  = argv[1];
    std::string output_path = (argc == 3) ? argv[2] : "";

    std::string source;
    try
    {
        source = read_file(input_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reading input: " << e.what() << "\n";
        return 1;
    }

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    std::string tmp_token_path = input_path + ".tokens.tmp";
    try
    {
        write_tokens_to_file(tokens, tmp_token_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing token file: " << e.what() << "\n";
        return 1;
    }

    std::unique_ptr<ParseTreeNode> parse_tree;
    try
    {
        TokenStream ts(tmp_token_path);
        Parser parser(ts);
        parse_tree = parser.parse_program();
    }
    catch (const std::exception &e)
    {
        std::remove(tmp_token_path.c_str());
        std::cerr << "Syntax Error: " << e.what() << "\n";
        return 1;
    }

    std::remove(tmp_token_path.c_str());

    // Cetak parse tree
    std::cout << "=== PARSE TREE ===\n";
    TreePrinter::printToConsole(*parse_tree);
    std::cout << "\n";

    ASTBuilder builder;
    std::unique_ptr<ASTNode> ast;
    try
    {
        ast = builder.build(*parse_tree);
    }
    catch (const std::exception &e)
    {
        std::cerr << "AST Build Error: " << e.what() << "\n";
        return 1;
    }

    SymbolTable sym;
    SemanticVisitor visitor(sym);

    try
    {
        visitor.visit(*ast);
    }
    catch (const SemanticError &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Semantic Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "=== DECORATED AST ===\n";
    ast->print(0);
    std::cout << "\n";

    std::cout << "=== SYMBOL TABLE ===\n";
    sym.print_all(std::cout);

    if (!output_path.empty())
    {
        std::ofstream out(output_path);
        if (!out.is_open())
        {
            std::cerr << "Cannot open output file: " << output_path << "\n";
            return 1;
        }
        out << "=== DECORATED AST ===\n";
        ast->print(0);
        out << "\n=== SYMBOL TABLE ===\n";
        sym.print_all(out);
        std::cout << "\nOutput written to: " << output_path << "\n";
    }

    return 0;
}