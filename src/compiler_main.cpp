#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.h"
#include "lexer/token.h"

#include "parser/Parser.hpp"
#include "parser/core/TokenStream.hpp"
#include "parser/core/ParseTreeNode.hpp"

#include "semantic/ASTBuilder.hpp"
#include "semantic/ast/DeclNodes.hpp"
#include "semantic/symbol_table/Symbol_Table.hpp"
#include "semantic/visitor/SemanticVisitor.hpp"
#include "semantic/error/SemanticError.hpp"

#include "codegen/CodegenVisitor.hpp"
#include "codegen/Instruction.hpp"

#include "interpreter/VirtualMachine.hpp"
#include "interpreter/VM_Exceptions.hpp"

namespace
{
std::string read_file(const std::string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_token(std::ostream &out, const Token &token)
{
    const std::string type_name = tokenTypeToString(token.type);
    switch (token.type)
    {
    case TokenType::INTCON:
    case TokenType::REALCON:
    case TokenType::CHARCON:
    case TokenType::STRING:
    case TokenType::IDENT:
    case TokenType::COMMENT:
    case TokenType::UNKNOWN:
    case TokenType::ERROR:
        out << type_name << " (" << token.value << ")\n";
        break;
    default:
        out << type_name << "\n";
        break;
    }
}

TokenStream tokens_to_stream(const std::vector<Token> &tokens)
{
    const std::string temp_path = ".arion_tokens.tmp";
    {
        std::ofstream temp(temp_path.c_str());
        if (!temp.is_open())
            throw std::runtime_error("Cannot create temporary token file");
        for (const Token &token : tokens)
            write_token(temp, token);
    }

    TokenStream ts(temp_path, true);
    std::remove(temp_path.c_str());
    return ts;
}

void print_usage(const char *program_name)
{
    std::cerr << "Arion Compiler - Full Pipeline (Milestone 4)\n"
              << "Usage:\n"
              << "  " << program_name << " <source.txt>\n"
              << "  " << program_name << " <source.txt> <output.txt>\n";
}
} // namespace

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    const std::string input_path = argv[1];
    const std::string output_path = (argc == 3) ? argv[2] : "";

    try
    {
        const std::string source = read_file(input_path);
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        TokenStream ts = tokens_to_stream(tokens);
        Parser parser(ts);
        std::unique_ptr<ParseTreeNode> parse_tree = parser.parse_program();

        ASTBuilder builder;
        std::unique_ptr<ASTNode> ast = builder.build(*parse_tree);

        ProgramNode *program = dynamic_cast<ProgramNode *>(ast.get());
        if (program == nullptr)
            throw std::runtime_error("AST root is not a ProgramNode");

        SymbolTable sym;
        SemanticVisitor semantic(sym);
        semantic.visit(*ast);

        CodegenVisitor codegen(sym);
        std::vector<Instruction> code = codegen.generate(*program);

        std::ostringstream report;
        report << "=== INTERMEDIATE CODE ===\n";
        print_instructions(report, code);

        VirtualMachine vm(code);
        std::string runtime_error;
        try
        {
            vm.run();
        }
        catch (const VMError &e)
        {
            runtime_error = e.what();
        }

        report << "\n=== OUTPUT ===\n";
        report << vm.output();
        if (!vm.output().empty() && vm.output().back() != '\n')
            report << "\n";
        if (!runtime_error.empty())
            report << "Runtime Error: " << runtime_error << "\n";

        std::cout << report.str();

        if (!output_path.empty())
        {
            std::ofstream out(output_path.c_str());
            if (!out.is_open())
                throw std::runtime_error("Cannot open output file: " + output_path);
            out << report.str();
            std::cout << "\nOutput written to: " << output_path << "\n";
        }

        if (!runtime_error.empty())
            return 1;
    }
    catch (const SemanticError &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
