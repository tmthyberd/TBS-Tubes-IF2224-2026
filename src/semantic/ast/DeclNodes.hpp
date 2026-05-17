#ifndef DECL_NODES_HPP
#define DECL_NODES_HPP

#include <memory>
#include <string>
#include <vector>
#include "ASTNode.hpp"

struct ProgramNode : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> declarations;
    std::unique_ptr<ASTNode> body;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct VarDeclNode : public ASTNode
{
    std::vector<std::string> names;
    std::unique_ptr<ASTNode> type_node;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct ConstDeclNode : public ASTNode
{
    std::string name;
    std::unique_ptr<ASTNode> value;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct TypeDeclNode : public ASTNode
{
    std::string name;
    std::unique_ptr<ASTNode> type_def;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct ParamGroupNode : public ASTNode
{
    std::vector<std::string> names;
    std::unique_ptr<ASTNode> type_node;
    bool is_var_param = false;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

// ProcDeclNode: deklarasi prosedur
struct ProcDeclNode : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> params;
    std::unique_ptr<ASTNode> body;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct FuncDeclNode : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> params;
    std::unique_ptr<ASTNode> return_type;
    std::unique_ptr<ASTNode> body;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct ArrayTypeNode : public ASTNode
{
    std::unique_ptr<ASTNode> index_range;
    std::unique_ptr<ASTNode> elem_type;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct RecordTypeNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> fields;
    bool is_anonymous = true;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct SubrangeNode : public ASTNode
{
    std::unique_ptr<ASTNode> low;
    std::unique_ptr<ASTNode> high;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

struct EnumNode : public ASTNode
{
    std::vector<std::string> values;

    std::string node_type() const override;
    void print(int indent = 0) const override;
};

#endif