#ifndef SEMANTIC_ERROR_HPP
#define SEMANTIC_ERROR_HPP

#include <stdexcept>
#include <string>

class SemanticError : public std::runtime_error
{
public:
    SemanticError(const std::string &message, int line, int col = -1)
        : std::runtime_error(format(message, line, col)),
          line(line), col(col) {}

    int line;
    int col;

private:
    static std::string format(const std::string &message, int line, int col)
    {
        std::string result = "Semantic Error";
        if (line >= 0)
            result += " [line " + std::to_string(line) + "]";
        if (col >= 0)
            result += " [col " + std::to_string(col) + "]";
        result += ": " + message;
        return result;
    }
};

#endif