#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "lexer.hpp"
#include "token.hpp"

/**
 * KERANGKA (SKELETON) - Main Program untuk Pascal Lexer
 * 
 * PENGGUNAAN:
 *   ./lexer <input.txt> <output.txt>
 * 
 * CONTOH:
 *   ./lexer test/input.txt test/output.txt
 */

int main(int argc, char* argv[]) {
    // =============================================
    // ARGUMENT VALIDATION - Sudah lengkap
    // =============================================
    if (argc < 3) {
        std::cerr << "Usage: ./lexer <input.txt> <output.txt>\n";
        return 1;
    }

    // =============================================
    // READ INPUT FILE - Sudah lengkap
    // =============================================
    std::ifstream inFile(argv[1]);
    if (!inFile) {
        std::cerr << "Error: cannot open input file: " << argv[1] << "\n";
        return 1;
    }

    // Baca seluruh file ke string
    std::stringstream buffer;
    buffer << inFile.rdbuf();

    // =============================================
    // TOKENIZE - Sudah lengkap
    // =============================================
    Lexer lexer(buffer.str());
    
    // false = tidak include komentar di output
    std::vector<Token> tokens = lexer.tokenize(false);

    // =============================================
    // WRITE OUTPUT FILE - Sudah lengkap
    // =============================================
    std::ofstream outFile(argv[2]);
    if (!outFile) {
        std::cerr << "Error: cannot open output file: " << argv[2] << "\n";
        return 1;
    }

    // Tulis setiap token ke stdout dan file output
    for (const Token& token : tokens) {
        std::string out = tokenToOutputString(token);
        std::cout << out << '\n';    // ke terminal
        outFile << out << '\n';       // ke file
    }

    return 0;
}
