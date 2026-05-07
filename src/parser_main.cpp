#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

#include "parser/Parser.hpp"
#include "parser/core/TokenStream.hpp"
#include "parser/core/TreePrinter.hpp"

void printUsage(const char *programName)
{
    std::cout << "Arion Language Parser" << std::endl;
    std::cout << "IF2224 - Teori Bahasa Formal dan Automata" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << programName
              << " <token_file.txt> [output_file.txt]" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  token_file.txt   Token file produced by the lexer"
              << std::endl;
    std::cout << "  output_file.txt  (optional) File to write the parse tree to"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " tokens.txt" << std::endl;
    std::cout << "  " << programName << " tokens.txt parse_tree.txt" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3)
    {
        printUsage(argv[0]);
        return 1;
    }

    const std::string tokenFilePath = argv[1];

    try
    {
        TokenStream ts(tokenFilePath, /*skipComments=*/true);

        Parser parser(ts);
        std::unique_ptr<ParseTreeNode> tree = parser.parse_program();

        if (argc == 3)
        {
            const std::string outputPath = argv[2];
            TreePrinter::printToFile(*tree, outputPath);
            std::cout << "Parse tree written to '" << outputPath << "'" << std::endl;
        }
        else
        {
            TreePrinter::printToConsole(*tree);
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}