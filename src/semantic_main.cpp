#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "semantic/ASTBuilder.hpp"
#include "semantic/symbol_table/Symbol_Table.hpp"
#include "semantic/visitor/SemanticVisitor.hpp"
#include "semantic/error/SemanticError.hpp"

#include "parser/core/ParseTreeNode.hpp"

namespace
{
// UTF-8 byte sequences used by the parser's TreePrinter.
const char *PIPE_INDENT  = "\xE2\x94\x82   ";                    // "│   " (6 bytes)
const char *BLANK_INDENT = "    ";                                // 4 spaces
const char *BRANCH_GLYPH = "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 "; // "├── " (10 bytes)
const char *LAST_GLYPH   = "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 "; // "└── " (10 bytes)

bool starts_with(const std::string &s, std::size_t pos, const char *prefix)
{
    std::size_t len = std::strlen(prefix);
    if (pos + len > s.size()) return false;
    return s.compare(pos, len, prefix) == 0;
}

void rstrip(std::string &s)
{
    while (!s.empty() && (s.back() == '\r' || s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
}

// Parse the textual parse-tree dump produced by parser/core/TreePrinter back
// into a ParseTreeNode hierarchy. Indentation uses "│   " (6 bytes) or "    "
// (4 bytes); the line connector is "├── " or "└── " (10 bytes each).
std::unique_ptr<ParseTreeNode> parse_parse_tree(std::istream &in)
{
    std::unique_ptr<ParseTreeNode> root;
    std::vector<ParseTreeNode *> stack; // stack[d] = current node at depth d

    std::string line;
    int line_no = 0;
    while (std::getline(in, line))
    {
        ++line_no;
        rstrip(line);
        if (line.empty()) continue;

        // Count indentation units, then look for the branch glyph.
        std::size_t i = 0;
        int prefix_units = 0;
        while (true)
        {
            if (starts_with(line, i, PIPE_INDENT))      { i += 6; ++prefix_units; }
            else if (starts_with(line, i, BLANK_INDENT)) { i += 4; ++prefix_units; }
            else break;
        }

        bool has_connector = false;
        if (starts_with(line, i, BRANCH_GLYPH) || starts_with(line, i, LAST_GLYPH))
        {
            i += 10;
            has_connector = true;
        }

        std::string name = line.substr(i);
        rstrip(name);
        if (name.empty()) continue;

        int depth = prefix_units + (has_connector ? 1 : 0);

        if (depth == 0)
        {
            if (root)
                throw std::runtime_error(
                    "Malformed parse tree at line " + std::to_string(line_no) +
                    ": multiple root nodes");
            root.reset(new ParseTreeNode(name));
            stack.clear();
            stack.push_back(root.get());
            continue;
        }

        if (!root)
            throw std::runtime_error(
                "Malformed parse tree at line " + std::to_string(line_no) +
                ": child line before root");
        if (static_cast<std::size_t>(depth) > stack.size())
            throw std::runtime_error(
                "Malformed parse tree at line " + std::to_string(line_no) +
                ": indentation skips levels");

        ParseTreeNode *parent = stack[depth - 1];
        ParseTreeNode &child = parent->addChild(name);
        if (static_cast<std::size_t>(depth) < stack.size())
            stack.resize(depth);
        stack.push_back(&child);
    }

    if (!root)
        throw std::runtime_error("Parse tree file is empty");
    return root;
}

void print_usage(const char *prog_name)
{
    std::cerr << "Arion Compiler - Semantic Analysis (Milestone 3)\n"
              << "Usage:\n"
              << "  " << prog_name << " <parse_tree.txt>\n"
              << "  " << prog_name << " <parse_tree.txt> <output.txt>\n";
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

    std::unique_ptr<ParseTreeNode> parse_tree;
    try
    {
        std::ifstream in(input_path.c_str());
        if (!in.is_open())
            throw std::runtime_error("Cannot open file: " + input_path);
        parse_tree = parse_parse_tree(in);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

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
    ast->print(std::cout);
    std::cout << "\n";

    std::cout << "=== SYMBOL TABLE ===\n";
    sym.print_all(std::cout);

    if (!output_path.empty())
    {
        std::ofstream out(output_path.c_str());
        if (!out.is_open())
        {
            std::cerr << "Cannot open output file: " << output_path << "\n";
            return 1;
        }
        out << "=== DECORATED AST ===\n";
        ast->print(out);
        out << "\n=== SYMBOL TABLE ===\n";
        sym.print_all(out);
        std::cout << "\nOutput written to: " << output_path << "\n";
    }

    return 0;
}
