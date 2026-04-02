#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "token.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        exit(1);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

void printToken(const Token& token) {
    std::string typeName = tokenTypeToString(token.type);
    
    switch (token.type) {
        // Token dengan nilai
        case TokenType::INTCON:
        case TokenType::REALCON:
        case TokenType::CHARCON:
        case TokenType::STRING:
        case TokenType::IDENT:
        case TokenType::COMMENT:
        case TokenType::UNKNOWN:
        case TokenType::ERROR:
            std::cout << typeName << " (" << token.value << ")" << std::endl;
            break;
            
        // Token tanpa nilai (keyword dan operator)
        default:
            std::cout << typeName << std::endl;
            break;
    }
}

void printUsage(const char* programName) {
    std::cout << "Arion Language Lexer" << std::endl;
    std::cout << "IF2224 - Teori Bahasa Formal dan Automata" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << programName << " <input_file.txt>" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " program.txt" << std::endl;
}


int main(int argc, char* argv[]) {
    // Cek argumen command line
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Baca file input
    std::string filename = argv[1];
    std::string sourceCode = readFile(filename);
    
    // Inisialisasi lexer
    Lexer lexer(sourceCode);
    
    // Tokenisasi dan cetak hasil
    std::vector<Token> tokens = lexer.tokenize();
    
    for (const Token& token : tokens) {
        printToken(token);
    }
    
    return 0;
}
