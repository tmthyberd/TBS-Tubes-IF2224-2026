#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include <memory>
#include <string>
#include "ast/ASTNode.hpp"
#include "ast/ExprNodes.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/DeclNodes.hpp"
#include "../parser/core/ParseTreeNode.hpp"

class ASTBuilder
{
public:
    std::unique_ptr<ASTNode> build(const ParseTreeNode &root);

private:
    // Program & Block
    std::unique_ptr<ASTNode> build_program(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_block(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_compound_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_statement_list(const ParseTreeNode &n);

    //  Deklarasi
    std::unique_ptr<ASTNode> build_declaration_part(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_const_declaration(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_type_declaration(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_var_declaration(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_subprogram_declaration(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_procedure_declaration(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_function_declaration(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_formal_parameter_list(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_parameter_group(const ParseTreeNode &n);

    // Tipe
    std::unique_ptr<ASTNode> build_type(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_array_type(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_record_type(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_range(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_enumerated(const ParseTreeNode &n);

    // Statement
    std::unique_ptr<ASTNode> build_assignment_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_if_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_while_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_for_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_repeat_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_case_statement(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_procedure_function_call(const ParseTreeNode &n);

    // Ekspresi
    std::unique_ptr<ASTNode> build_expression(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_simple_expression(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_term(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_factor(const ParseTreeNode &n);
    std::unique_ptr<ASTNode> build_variable(const ParseTreeNode &n);

    // Helper
    bool is_node(const ParseTreeNode &n, const std::string &name) const;
    bool is_token(const ParseTreeNode &n, const std::string &token_type) const;
    const ParseTreeNode &child(const ParseTreeNode &n, int i) const;
    int child_count(const ParseTreeNode &n) const;
    std::string token_value(const ParseTreeNode &n) const;
};

#endif
