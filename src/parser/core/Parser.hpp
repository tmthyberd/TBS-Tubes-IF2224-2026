#ifndef PARSER_HPP
#define PARSER_HPP

#include <memory>
#include "core/TokenStream.hpp"
#include "core/ParseTreeNode.hpp"

class Parser
{
private:
    TokenStream ts;

public:
    explicit Parser(const TokenStream &tokenStream) : ts(tokenStream) {}

    std::unique_ptr<ParseTreeNode> parse_program();
    std::unique_ptr<ParseTreeNode> parse_program_header();
    std::unique_ptr<ParseTreeNode> parse_declaration_part();
    std::unique_ptr<ParseTreeNode> parse_const_declaration();
    std::unique_ptr<ParseTreeNode> parse_constant();
    std::unique_ptr<ParseTreeNode> parse_type_declaration();
    std::unique_ptr<ParseTreeNode> parse_var_declaration();
    std::unique_ptr<ParseTreeNode> parse_identifier_list();
    std::unique_ptr<ParseTreeNode> parse_type();
    std::unique_ptr<ParseTreeNode> parse_array_type();
    std::unique_ptr<ParseTreeNode> parse_range();
    std::unique_ptr<ParseTreeNode> parse_enumerated();
    std::unique_ptr<ParseTreeNode> parse_record_type();
    std::unique_ptr<ParseTreeNode> parse_field_list();
    std::unique_ptr<ParseTreeNode> parse_field_part();
    std::unique_ptr<ParseTreeNode> parse_subprogram_declaration();
    std::unique_ptr<ParseTreeNode> parse_procedure_declaration();
    std::unique_ptr<ParseTreeNode> parse_function_declaration();
    std::unique_ptr<ParseTreeNode> parse_block();
    std::unique_ptr<ParseTreeNode> parse_formal_parameter_list();
    std::unique_ptr<ParseTreeNode> parse_parameter_group();

    std::unique_ptr<ParseTreeNode> parse_compound_statement();
    std::unique_ptr<ParseTreeNode> parse_statement_list();
    std::unique_ptr<ParseTreeNode> parse_statement();
    std::unique_ptr<ParseTreeNode> parse_assignment_statement();
    std::unique_ptr<ParseTreeNode> parse_if_statement();
    std::unique_ptr<ParseTreeNode> parse_case_statement();
    std::unique_ptr<ParseTreeNode> parse_case_block();
    std::unique_ptr<ParseTreeNode> parse_while_statement();
    std::unique_ptr<ParseTreeNode> parse_repeat_statement();
    std::unique_ptr<ParseTreeNode> parse_for_statement();
    std::unique_ptr<ParseTreeNode> parse_procedure_function_call();

    std::unique_ptr<ParseTreeNode> parse_variable();
    std::unique_ptr<ParseTreeNode> parse_component_variable();
    std::unique_ptr<ParseTreeNode> parse_index_list();
    std::unique_ptr<ParseTreeNode> parse_parameter_list();
    std::unique_ptr<ParseTreeNode> parse_expression();
    std::unique_ptr<ParseTreeNode> parse_simple_expression();
    std::unique_ptr<ParseTreeNode> parse_term();
    std::unique_ptr<ParseTreeNode> parse_factor();

    std::unique_ptr<ParseTreeNode> parse_relational_operator();
    std::unique_ptr<ParseTreeNode> parse_additive_operator();
    std::unique_ptr<ParseTreeNode> parse_multiplicative_operator();
};

#endif