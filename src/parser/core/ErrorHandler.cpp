#include "ErrorHandler.hpp"
#include <sstream>

// Token Formatting Helper

namespace {
bool tokenTypeCarriesValue(TokenType type) {
    switch (type) {
        case TokenType::INTCON:
        case TokenType::REALCON:
        case TokenType::CHARCON:
        case TokenType::STRING:
        case TokenType::IDENT:
        case TokenType::COMMENT:
        case TokenType::UNKNOWN:
        case TokenType::ERROR:
            return true;
        default:
            return false;
    }
}
}

// Exception Construction

SyntaxError::SyntaxError(const std::string& message)
    : std::runtime_error(message) {}

// Syntax Error Helpers

void ErrorHandler::syntaxError(const std::string& message) {
    throw SyntaxError("Syntax error: " + message);
}

void ErrorHandler::unexpectedToken(const Token& actual,
                                   TokenType expected,
                                   const std::string& context) {
    unexpectedToken(actual, tokenTypeToString(expected), context);
}

void ErrorHandler::unexpectedToken(const Token& actual,
                                   const std::string& expected,
                                   const std::string& context) {
    throw SyntaxError(buildUnexpectedMessage(actual, expected, context));
}

// Message Formatting

std::string ErrorHandler::formatToken(const Token& token) {
    std::ostringstream out;
    out << tokenTypeToString(token.type);

    if (tokenTypeCarriesValue(token.type)) {
        out << "(" << token.value << ")";
    }

    return out.str();
}

std::string ErrorHandler::buildUnexpectedMessage(const Token& actual,
                                                 const std::string& expected,
                                                 const std::string& context) {
    std::ostringstream out;
    out << "Syntax error: unexpected token " << formatToken(actual);

    if (!expected.empty()) {
        out << ", expected " << expected;
    }

    if (actual.line > 0 && actual.column > 0) {
        out << " at line " << actual.line << ", column " << actual.column;
    }

    if (!context.empty()) {
        out << " while parsing " << context;
    }

    return out.str();
}
