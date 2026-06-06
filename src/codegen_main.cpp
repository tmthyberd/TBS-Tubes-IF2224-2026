#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "semantic/ast/DeclNodes.hpp"
#include "semantic/symbol_table/Symbol_Table.hpp"

#include "codegen/DASTReader.hpp"
#include "codegen/CodegenVisitor.hpp"
#include "codegen/Instruction.hpp"

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

void print_usage(const char *program_name)
{
    std::cerr << "Arion Code Generator - Masukan Decorated AST (Milestone 4)\n"
              << "Usage:\n"
              << "  " << program_name << " <decorated_ast.txt>\n"
              << "  " << program_name << " <decorated_ast.txt> <output.txt>\n";
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
        const std::string text = read_file(input_path);

        SymbolTable sym;
        std::unique_ptr<ASTNode> ast = load_decorated_ast(text, sym);

        ProgramNode *program = dynamic_cast<ProgramNode *>(ast.get());
        if (program == nullptr)
            throw std::runtime_error("Decorated AST root is not a ProgramNode");

        CodegenVisitor codegen(sym);
        std::vector<Instruction> code = codegen.generate(*program);

        // Code generator hanya bertugas mengubah Decorated AST menjadi
        // Intermediate Code. Eksekusi/output adalah tugas Interpreter
        // (tersedia melalui executable `compiler`), sehingga di sini tidak ada
        // bagian "=== OUTPUT ===".
        std::ostringstream report;
        report << "=== INTERMEDIATE CODE ===\n";
        print_instructions(report, code);

        std::cout << report.str();

        if (!output_path.empty())
        {
            std::ofstream out(output_path.c_str());
            if (!out.is_open())
                throw std::runtime_error("Cannot open output file: " + output_path);
            out << report.str();
            std::cout << "\nOutput written to: " << output_path << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
