#ifndef EXPR_NODES_HPP
#define EXPR_NODES_HPP

#include <memory>
#include <string>
#include <vector>
#include "ASTNode.hpp"

struct BinOpNode : public ASTNode
{
    std::string op; // "+", "-", "*", "/", "div", "mod",
                    // "and", "or", "==", "<>", "<", ">", "<=", ">="
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

// UnaryOpNode: ekspresi unari, e.g. not x, -n
struct UnaryOpNode : public ASTNode
{
    std::string op; // "not", "-"
    std::unique_ptr<ASTNode> operand;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct VarNode : public ASTNode
{
    std::string name;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct NumberNode : public ASTNode
{
    int value;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct RealNode : public ASTNode
{
    double value;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct CharNode : public ASTNode
{
    char value;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct StringNode : public ASTNode
{
    std::string value;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct BoolNode : public ASTNode
{
    bool value;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct ArrayAccessNode : public ASTNode
{
    std::unique_ptr<ASTNode> array;
    std::vector<std::unique_ptr<ASTNode>> indices;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct RecordAccessNode : public ASTNode
{
    std::unique_ptr<ASTNode> record;
    std::string field_name;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct FuncCallExprNode : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

#endif