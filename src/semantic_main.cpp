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

static void print_ast(const ASTNode &n, std::ostream &out, int indent = 0)
{
    n.print(indent); 
}


int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    std::string input_path = argv[1];

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

    TokenStream ts(input_path);

    std::unique_ptr<ParseTreeNode> parse_tree;
    try
    {
        Parser parser(ts);
        parse_tree = parser.parse_program();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Syntax Error: " << e.what() << "\n";
        return 1;
    }

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

    if (argc == 3)
    {
        std::ofstream out(argv[2]);
        if (!out.is_open())
        {
            std::cerr << "Cannot open output file: " << argv[2] << "\n";
            return 1;
        }
        out << "=== DECORATED AST ===\n";
        ast->print(0);   // TODO: redirect print() ke out
        out << "\n=== SYMBOL TABLE ===\n";
        sym.print_all(out);
        std::cout << "\nOutput written to: " << argv[2] << "\n";
    }

    return 0;
}