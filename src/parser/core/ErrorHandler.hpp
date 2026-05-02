#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <stdexcept>
#include <string>
#include "../../lexer/token.h"

class SyntaxError : public std::runtime_error {
public:
    explicit SyntaxError(const std::string& message);
};

class ErrorHandler {
public:
    static void syntaxError(const std::string& message);
    static void unexpectedToken(const Token& actual,
                                TokenType expected,
                                const std::string& context = "");
    static void unexpectedToken(const Token& actual,
                                const std::string& expected,
                                const std::string& context = "");

private:
    static std::string formatToken(const Token& token);
    static std::string buildUnexpectedMessage(const Token& actual,
                                              const std::string& expected,
                                              const std::string& context);
};

#endif
