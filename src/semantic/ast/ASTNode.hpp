#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <string>

enum class TypeCode
{
    UNKNOWN = -1,
    INTEGER = 0,
    REAL = 1,
    CHAR = 2,
    BOOLEAN = 3,
    STRING = 4,
    ARRAY = 5,
    RECORD = 6,
    SUBRANGE = 7,
    VOID = 8,
    ENUM = 9
};

enum class ObjClass
{
    CONSTANT,
    VARIABLE,
    TYPE,
    PROCEDURE,
    FUNCTION,
    RESERVED,
    PROGRAM
};

class ASTNode
{
public:
    TypeCode type_code = TypeCode::UNKNOWN;
    int tab_index = -1; // index ke SymbolTable::tab
    int lev = -1;       // lexical level
    int line = -1;      // baris di source, untuk error reporting

    virtual ~ASTNode() = default;
    virtual std::string node_type() const = 0;
    virtual void print(int indent = 0) const = 0;
};

#endif
