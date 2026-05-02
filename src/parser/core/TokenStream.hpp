#ifndef TOKEN_STREAM_HPP
#define TOKEN_STREAM_HPP

#include <cstddef>
#include <string>
#include <vector>
#include "../../lexer/token.h"

class TokenStream {
public:
    TokenStream();
    explicit TokenStream(const std::string& tokenFilePath, bool skipComments = true);

    void loadFromFile(const std::string& tokenFilePath, bool skipComments = true);

    const Token& peek(std::size_t offset = 0) const;
    const Token& advance();
    const Token& previous() const;

    bool check(TokenType expected) const;
    bool match(TokenType expected);
    const Token& expect(TokenType expected, const std::string& context = "");

    bool isAtEnd() const;

private:
    std::vector<Token> tokens;
    std::size_t currentIndex;

    void appendEndToken();
    static Token parseTokenLine(const std::string& line, int lineNumber);
    static TokenType tokenTypeFromString(const std::string& typeName);
    static std::string trim(const std::string& value);
    static std::string rtrim(const std::string& value);
    static std::string toLower(const std::string& value);
};

#endif
