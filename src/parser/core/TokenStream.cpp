#include "TokenStream.hpp"
#include "ErrorHandler.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

// Constructor & Loading

TokenStream::TokenStream()
    : tokens(), currentIndex(0) {
    appendEndToken();
}

TokenStream::TokenStream(const std::string& tokenFilePath, bool skipComments)
    : tokens(), currentIndex(0) {
    loadFromFile(tokenFilePath, skipComments);
}

void TokenStream::loadFromFile(const std::string& tokenFilePath, bool skipComments) {
    std::ifstream file(tokenFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open token file '" + tokenFilePath + "'");
    }

    tokens.clear();
    currentIndex = 0;

    std::string line;
    int lineNumber = 1;
    while (std::getline(file, line)) {
        line = rtrim(line);

        if (!trim(line).empty()) {
            Token token = parseTokenLine(line, lineNumber);
            if (!(skipComments && token.type == TokenType::COMMENT)) {
                tokens.push_back(token);
            }
        }

        ++lineNumber;
    }

    appendEndToken();
}

// Token Navigation

const Token& TokenStream::peek(std::size_t offset) const {
    std::size_t target = currentIndex + offset;
    if (target >= tokens.size()) {
        return tokens.back();
    }

    return tokens[target];
}

const Token& TokenStream::advance() {
    if (!isAtEnd()) {
        ++currentIndex;
    }

    return previous();
}

const Token& TokenStream::previous() const {
    if (currentIndex == 0) {
        return tokens.front();
    }

    return tokens[currentIndex - 1];
}

// Matching Helpers

bool TokenStream::check(TokenType expected) const {
    return peek().type == expected;
}

bool TokenStream::match(TokenType expected) {
    if (!check(expected)) {
        return false;
    }

    advance();
    return true;
}

const Token& TokenStream::expect(TokenType expected, const std::string& context) {
    if (!check(expected)) {
        ErrorHandler::unexpectedToken(peek(), expected, context);
    }

    return advance();
}

// Stream State

bool TokenStream::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

// EOF Handling

void TokenStream::appendEndToken() {
    if (!tokens.empty() && tokens.back().type == TokenType::END_OF_FILE) {
        return;
    }

    int line = tokens.empty() ? 1 : tokens.back().line;
    int column = tokens.empty() ? 1 : tokens.back().column;
    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
}

// Token File Parsing

Token TokenStream::parseTokenLine(const std::string& line, int lineNumber) {
    std::string cleaned = rtrim(line);
    std::string typeName;
    std::string value;

    std::size_t openParen = cleaned.find('(');
    std::size_t closeParen = cleaned.rfind(')');

    if (openParen != std::string::npos) {
        if (closeParen == std::string::npos || closeParen < openParen) {
            throw std::runtime_error("Malformed token line " + std::to_string(lineNumber) + ": " + line);
        }

        typeName = trim(cleaned.substr(0, openParen));
        value = cleaned.substr(openParen + 1, closeParen - openParen - 1);
    } else {
        typeName = trim(cleaned);
    }

    if (typeName.empty()) {
        throw std::runtime_error("Missing token type at line " + std::to_string(lineNumber));
    }

    return Token(tokenTypeFromString(typeName), value, lineNumber, 1);
}

TokenType TokenStream::tokenTypeFromString(const std::string& typeName) {
    std::string lower = toLower(typeName);

    if (lower == "intcon") return TokenType::INTCON;
    if (lower == "realcon") return TokenType::REALCON;
    if (lower == "charcon") return TokenType::CHARCON;
    if (lower == "string") return TokenType::STRING;
    if (lower == "notsy") return TokenType::NOTSY;
    if (lower == "andsy") return TokenType::ANDSY;
    if (lower == "orsy") return TokenType::ORSY;
    if (lower == "plus") return TokenType::PLUS;
    if (lower == "minus") return TokenType::MINUS;
    if (lower == "times") return TokenType::TIMES;
    if (lower == "idiv") return TokenType::IDIV;
    if (lower == "rdiv") return TokenType::RDIV;
    if (lower == "imod") return TokenType::IMOD;
    if (lower == "eql") return TokenType::EQL;
    if (lower == "neq") return TokenType::NEQ;
    if (lower == "gtr") return TokenType::GTR;
    if (lower == "geq") return TokenType::GEQ;
    if (lower == "lss") return TokenType::LSS;
    if (lower == "leq") return TokenType::LEQ;
    if (lower == "lparent") return TokenType::LPARENT;
    if (lower == "rparent") return TokenType::RPARENT;
    if (lower == "lbrack") return TokenType::LBRACK;
    if (lower == "rbrack") return TokenType::RBRACK;
    if (lower == "comma") return TokenType::COMMA;
    if (lower == "semicolon") return TokenType::SEMICOLON;
    if (lower == "period") return TokenType::PERIOD;
    if (lower == "dot") return TokenType::PERIOD;
    if (lower == "colon") return TokenType::COLON;
    if (lower == "becomes") return TokenType::BECOMES;
    if (lower == "constsy") return TokenType::CONSTSY;
    if (lower == "typesy") return TokenType::TYPESY;
    if (lower == "varsy") return TokenType::VARSY;
    if (lower == "functionsy") return TokenType::FUNCTIONSY;
    if (lower == "proceduresy") return TokenType::PROCEDURESY;
    if (lower == "arraysy") return TokenType::ARRAYSY;
    if (lower == "recordsy") return TokenType::RECORDSY;
    if (lower == "programsy") return TokenType::PROGRAMSY;
    if (lower == "ident") return TokenType::IDENT;
    if (lower == "beginsy") return TokenType::BEGINSY;
    if (lower == "ifsy") return TokenType::IFSY;
    if (lower == "casesy") return TokenType::CASESY;
    if (lower == "repeatsy") return TokenType::REPEATSY;
    if (lower == "whilesy") return TokenType::WHILESY;
    if (lower == "forsy") return TokenType::FORSY;
    if (lower == "endsy") return TokenType::ENDSY;
    if (lower == "elsesy" || lower == "elsy") return TokenType::ELSESY;
    if (lower == "untilsy") return TokenType::UNTILSY;
    if (lower == "ofsy") return TokenType::OFSY;
    if (lower == "dosy") return TokenType::DOSY;
    if (lower == "tosy") return TokenType::TOSY;
    if (lower == "downtosy") return TokenType::DOWNTOSY;
    if (lower == "thensy") return TokenType::THENSY;
    if (lower == "comment") return TokenType::COMMENT;
    if (lower == "unknown") return TokenType::UNKNOWN;
    if (lower == "error") return TokenType::ERROR;
    if (lower == "eof" || lower == "end_of_file") return TokenType::END_OF_FILE;

    throw std::runtime_error("Unknown token type '" + typeName + "'");
}

// String Helpers

std::string TokenStream::trim(const std::string& value) {
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    return value.substr(first, last - first);
}

std::string TokenStream::rtrim(const std::string& value) {
    std::size_t last = value.size();
    while (last > 0 && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    return value.substr(0, last);
}

std::string TokenStream::toLower(const std::string& value) {
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}
