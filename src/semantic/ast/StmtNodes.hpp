#ifndef STMT_NODES_HPP
#define STMT_NODES_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "ASTNode.hpp"

struct BlockNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;
    int btab_index = -1;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct AssignNode : public ASTNode
{
    std::unique_ptr<ASTNode> target;
    std::unique_ptr<ASTNode> value;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct IfNode : public ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> then_branch;
    std::unique_ptr<ASTNode> else_branch;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct WhileNode : public ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct ForNode : public ASTNode
{
    std::string var_name;
    std::unique_ptr<ASTNode> from_expr;
    std::unique_ptr<ASTNode> to_expr;
    bool is_downto = false;
    std::unique_ptr<ASTNode> body;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct RepeatNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> body;
    std::unique_ptr<ASTNode> condition;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct CaseNode : public ASTNode
{
    std::unique_ptr<ASTNode> selector;
    std::vector<std::pair<std::unique_ptr<ASTNode>,
                          std::unique_ptr<ASTNode>>>
        cases;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

struct ProcCallNode : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    std::string node_type() const override;
    void print(std::ostream &out, const std::string &prefix = "",
               bool is_last = true, const std::string &role = "") const override;
};

#endif