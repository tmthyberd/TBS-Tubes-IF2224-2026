#include "ASTBuilder.hpp"
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
const char *INDENT_STEP = "  ";

inline std::string child_prefix(const std::string &p, bool /*is_last*/)
{
    return p + INDENT_STEP;
}

std::string typecode_name(TypeCode t)
{
    switch (t)
    {
    case TypeCode::INTEGER:  return "integer";
    case TypeCode::REAL:     return "real";
    case TypeCode::CHAR:     return "char";
    case TypeCode::BOOLEAN:  return "boolean";
    case TypeCode::STRING:   return "string";
    case TypeCode::ARRAY:    return "array";
    case TypeCode::RECORD:   return "record";
    case TypeCode::SUBRANGE: return "subrange";
    case TypeCode::ENUM:     return "enum";
    case TypeCode::VOID:     return "void";
    default:                 return "unknown";
    }
}

std::string annot_suffix(const ASTNode &n)
{
    std::string parts;
    auto add = [&](const std::string &s) {
        parts += parts.empty() ? "" : ", ";
        parts += s;
    };
    if (n.tab_index >= 0)             add("tab_index:" + std::to_string(n.tab_index));
    if (n.type_code != TypeCode::UNKNOWN) add("type:" + typecode_name(n.type_code));
    if (n.lev >= 0)                   add("lev:" + std::to_string(n.lev));
    return parts.empty() ? "" : (" [" + parts + "]");
}

void emit_line(std::ostream &out,
               const std::string &prefix,
               bool /*is_last*/,
               const std::string &role,
               const std::string &label,
               const std::string &suffix)
{
    out << prefix;
    if (!role.empty()) out << "[" << role << "] ";
    out << label << suffix << '\n';
}

std::string lower_text(const std::string &value)
{
    std::string result = value;
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

bool token_has_type(const ParseTreeNode &n, const std::string &type)
{
    std::size_t open = n.name.find('(');
    std::string name = open == std::string::npos ? n.name : n.name.substr(0, open);
    return lower_text(name) == lower_text(type);
}

std::string token_text(const ParseTreeNode &n)
{
    std::size_t open = n.name.find('(');
    std::size_t close = n.name.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close <= open)
        return "";
    return n.name.substr(open + 1, close - open - 1);
}

std::string token_type_name(const ParseTreeNode &n)
{
    std::size_t open = n.name.find('(');
    if (open == std::string::npos)
        return lower_text(n.name);
    return lower_text(n.name.substr(0, open));
}

std::string operator_text(const ParseTreeNode &n)
{
    const ParseTreeNode *token = &n;
    if (!n.children.empty())
        token = n.children.front().get();

    std::string op = token_type_name(*token);
    if (op == "plus") return "+";
    if (op == "minus") return "-";
    if (op == "times") return "*";
    if (op == "rdiv") return "/";
    if (op == "idiv") return "div";
    if (op == "imod") return "mod";
    if (op == "andsy") return "and";
    if (op == "orsy") return "or";
    if (op == "eql") return "==";
    if (op == "neq") return "<>";
    if (op == "lss") return "<";
    if (op == "gtr") return ">";
    if (op == "leq") return "<=";
    if (op == "geq") return ">=";
    if (op == "notsy") return "not";
    return op;
}

std::unique_ptr<ASTNode> make_literal_from_token(const ParseTreeNode &n)
{
    if (token_has_type(n, "intcon"))
    {
        std::unique_ptr<NumberNode> node(new NumberNode());
        node->value = std::atoi(token_text(n).c_str());
        node->type_code = TypeCode::INTEGER;
        return std::unique_ptr<ASTNode>(std::move(node));
    }
    if (token_has_type(n, "realcon"))
    {
        std::unique_ptr<RealNode> node(new RealNode());
        node->value = std::atof(token_text(n).c_str());
        node->type_code = TypeCode::REAL;
        return std::unique_ptr<ASTNode>(std::move(node));
    }
    if (token_has_type(n, "charcon"))
    {
        std::unique_ptr<CharNode> node(new CharNode());
        std::string value = token_text(n);
        node->value = value.empty() ? '\0' : value[0];
        node->type_code = TypeCode::CHAR;
        return std::unique_ptr<ASTNode>(std::move(node));
    }
    if (token_has_type(n, "string"))
    {
        std::unique_ptr<StringNode> node(new StringNode());
        node->value = token_text(n);
        node->type_code = TypeCode::STRING;
        return std::unique_ptr<ASTNode>(std::move(node));
    }
    if (token_has_type(n, "ident"))
    {
        std::string value = token_text(n);
        std::string lower = lower_text(value);
        if (lower == "true" || lower == "false")
        {
            std::unique_ptr<BoolNode> node(new BoolNode());
            node->value = lower == "true";
            node->type_code = TypeCode::BOOLEAN;
            return std::unique_ptr<ASTNode>(std::move(node));
        }

        std::unique_ptr<VarNode> node(new VarNode());
        node->name = value;
        return std::unique_ptr<ASTNode>(std::move(node));
    }

    return std::unique_ptr<ASTNode>();
}

void append_from_container(std::vector<std::unique_ptr<ASTNode> > &target,
                           std::unique_ptr<ASTNode> source)
{
    if (!source)
        return;

    BlockNode *block = dynamic_cast<BlockNode *>(source.get());
    if (block)
    {
        for (std::size_t i = 0; i < block->statements.size(); ++i)
            target.push_back(std::move(block->statements[i]));
        return;
    }

    target.push_back(std::move(source));
}
}

// AST node printing — Decorated AST with tree-drawing glyphs and per-node
// semantic annotations (tab_index / type / lev). Format follows the spec
// example in §II.E.

std::string ProgramNode::node_type() const { return "ProgramNode"; }
std::string VarDeclNode::node_type() const { return "VarDeclNode"; }
std::string ConstDeclNode::node_type() const { return "ConstDeclNode"; }
std::string TypeDeclNode::node_type() const { return "TypeDeclNode"; }
std::string ParamGroupNode::node_type() const { return "ParamGroupNode"; }
std::string ProcDeclNode::node_type() const { return "ProcDeclNode"; }
std::string FuncDeclNode::node_type() const { return "FuncDeclNode"; }
std::string ArrayTypeNode::node_type() const { return "ArrayTypeNode"; }
std::string RecordTypeNode::node_type() const { return "RecordTypeNode"; }
std::string SubrangeNode::node_type() const { return "SubrangeNode"; }
std::string EnumNode::node_type() const { return "EnumNode"; }
std::string BlockNode::node_type() const { return "BlockNode"; }
std::string AssignNode::node_type() const { return "AssignNode"; }
std::string IfNode::node_type() const { return "IfNode"; }
std::string WhileNode::node_type() const { return "WhileNode"; }
std::string ForNode::node_type() const { return "ForNode"; }
std::string RepeatNode::node_type() const { return "RepeatNode"; }
std::string CaseNode::node_type() const { return "CaseNode"; }
std::string ProcCallNode::node_type() const { return "ProcCallNode"; }
std::string BinOpNode::node_type() const { return "BinOpNode"; }
std::string UnaryOpNode::node_type() const { return "UnaryOpNode"; }
std::string VarNode::node_type() const { return "VarNode"; }
std::string NumberNode::node_type() const { return "NumberNode"; }
std::string RealNode::node_type() const { return "RealNode"; }
std::string CharNode::node_type() const { return "CharNode"; }
std::string StringNode::node_type() const { return "StringNode"; }
std::string BoolNode::node_type() const { return "BoolNode"; }
std::string ArrayAccessNode::node_type() const { return "ArrayAccessNode"; }
std::string RecordAccessNode::node_type() const { return "RecordAccessNode"; }
std::string FuncCallExprNode::node_type() const { return "FuncCallExprNode"; }

// --- Declarations ---------------------------------------------------------

void ProgramNode::print(std::ostream &out, const std::string &prefix,
                        bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    std::size_t total = declarations.size() + (body ? 1 : 0);
    std::size_t idx = 0;
    for (std::size_t i = 0; i < declarations.size(); ++i, ++idx)
        if (declarations[i])
            declarations[i]->print(out, cp, idx + 1 == total, "decl");
    if (body) body->print(out, cp, true, "body");
}

void VarDeclNode::print(std::ostream &out, const std::string &prefix,
                        bool is_last, const std::string &role) const
{
    std::string label = node_type() + "(names: ";
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        if (i) label += ", ";
        label += "'" + names[i] + "'";
    }
    label += ")";
    emit_line(out, prefix, is_last, role, label, annot_suffix(*this));
    if (type_node)
        type_node->print(out, child_prefix(prefix, is_last), true, "type");
}

void ConstDeclNode::print(std::ostream &out, const std::string &prefix,
                          bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    if (value)
        value->print(out, child_prefix(prefix, is_last), true, "value");
}

void TypeDeclNode::print(std::ostream &out, const std::string &prefix,
                         bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    if (type_def)
        type_def->print(out, child_prefix(prefix, is_last), true, "type");
}

void ParamGroupNode::print(std::ostream &out, const std::string &prefix,
                           bool is_last, const std::string &role) const
{
    std::string label = node_type() + "(";
    label += is_var_param ? "var: true, " : "var: false, ";
    label += "names: ";
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        if (i) label += ", ";
        label += "'" + names[i] + "'";
    }
    label += ")";
    emit_line(out, prefix, is_last, role, label, annot_suffix(*this));
    if (type_node)
        type_node->print(out, child_prefix(prefix, is_last), true, "type");
}

void ProcDeclNode::print(std::ostream &out, const std::string &prefix,
                         bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    std::size_t total = params.size() + (body ? 1 : 0);
    std::size_t idx = 0;
    for (std::size_t i = 0; i < params.size(); ++i, ++idx)
        if (params[i])
            params[i]->print(out, cp, idx + 1 == total, "param");
    if (body) body->print(out, cp, true, "body");
}

void FuncDeclNode::print(std::ostream &out, const std::string &prefix,
                         bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    std::size_t total = params.size() + (return_type ? 1 : 0) + (body ? 1 : 0);
    std::size_t idx = 0;
    for (std::size_t i = 0; i < params.size(); ++i, ++idx)
        if (params[i])
            params[i]->print(out, cp, idx + 1 == total, "param");
    if (return_type)
    {
        return_type->print(out, cp, idx + 1 == total, "returns");
        ++idx;
    }
    if (body) body->print(out, cp, true, "body");
}

void ArrayTypeNode::print(std::ostream &out, const std::string &prefix,
                          bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    bool has_elem = static_cast<bool>(elem_type);
    if (index_range)
        index_range->print(out, cp, !has_elem, "index");
    if (elem_type)
        elem_type->print(out, cp, true, "elem");
}

void RecordTypeNode::print(std::ostream &out, const std::string &prefix,
                           bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    for (std::size_t i = 0; i < fields.size(); ++i)
        if (fields[i])
            fields[i]->print(out, cp, i + 1 == fields.size(), "field");
}

void SubrangeNode::print(std::ostream &out, const std::string &prefix,
                         bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    bool has_high = static_cast<bool>(high);
    if (low)  low->print(out, cp, !has_high, "low");
    if (high) high->print(out, cp, true, "high");
}

void EnumNode::print(std::ostream &out, const std::string &prefix,
                     bool is_last, const std::string &role) const
{
    std::string label = node_type() + "(values: ";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i) label += ", ";
        label += "'" + values[i] + "'";
    }
    label += ")";
    emit_line(out, prefix, is_last, role, label, annot_suffix(*this));
}

// --- Statements -----------------------------------------------------------

void BlockNode::print(std::ostream &out, const std::string &prefix,
                      bool is_last, const std::string &role) const
{
    std::string label = node_type();
    if (btab_index >= 0)
        label += "(btab_index: " + std::to_string(btab_index) + ")";
    emit_line(out, prefix, is_last, role, label, annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    for (std::size_t i = 0; i < statements.size(); ++i)
        if (statements[i])
            statements[i]->print(out, cp, i + 1 == statements.size(), "stmt");
}

void AssignNode::print(std::ostream &out, const std::string &prefix,
                       bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    if (target) target->print(out, cp, !value, "target");
    if (value)  value->print(out, cp, true, "value");
}

void IfNode::print(std::ostream &out, const std::string &prefix,
                   bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    bool has_then = static_cast<bool>(then_branch);
    bool has_else = static_cast<bool>(else_branch);
    if (condition)   condition->print(out, cp, !has_then && !has_else, "cond");
    if (then_branch) then_branch->print(out, cp, !has_else, "then");
    if (else_branch) else_branch->print(out, cp, true, "else");
}

void WhileNode::print(std::ostream &out, const std::string &prefix,
                      bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    if (condition) condition->print(out, cp, !body, "cond");
    if (body)      body->print(out, cp, true, "body");
}

void ForNode::print(std::ostream &out, const std::string &prefix,
                    bool is_last, const std::string &role) const
{
    std::string label = node_type() + "(var: '" + var_name +
                        "', dir: " + (is_downto ? "downto" : "to") + ")";
    emit_line(out, prefix, is_last, role, label, annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    bool has_to = static_cast<bool>(to_expr);
    bool has_body = static_cast<bool>(body);
    if (from_expr) from_expr->print(out, cp, !has_to && !has_body, "from");
    if (to_expr)   to_expr->print(out, cp, !has_body, "to");
    if (body)      body->print(out, cp, true, "body");
}

void RepeatNode::print(std::ostream &out, const std::string &prefix,
                       bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    std::size_t total = body.size() + (condition ? 1 : 0);
    std::size_t idx = 0;
    for (std::size_t i = 0; i < body.size(); ++i, ++idx)
        if (body[i])
            body[i]->print(out, cp, idx + 1 == total, "stmt");
    if (condition) condition->print(out, cp, true, "until");
}

void CaseNode::print(std::ostream &out, const std::string &prefix,
                     bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    bool has_cases = !cases.empty();
    if (selector) selector->print(out, cp, !has_cases, "selector");
    for (std::size_t i = 0; i < cases.size(); ++i)
    {
        bool last_case = (i + 1 == cases.size());
        // Print "case" group as: label then stmt nested under it.
        if (cases[i].first)
            cases[i].first->print(out, cp, last_case && !cases[i].second, "label");
        if (cases[i].second)
        {
            std::string cp2 = child_prefix(cp, last_case);
            cases[i].second->print(out, cp2, true, "stmt");
        }
    }
}

void ProcCallNode::print(std::ostream &out, const std::string &prefix,
                         bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    for (std::size_t i = 0; i < args.size(); ++i)
        if (args[i])
            args[i]->print(out, cp, i + 1 == args.size(), "arg");
}

// --- Expressions ----------------------------------------------------------

void BinOpNode::print(std::ostream &out, const std::string &prefix,
                      bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(op: '" + op + "')", annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    if (left)  left->print(out, cp, !right, "left");
    if (right) right->print(out, cp, true, "right");
}

void UnaryOpNode::print(std::ostream &out, const std::string &prefix,
                        bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(op: '" + op + "')", annot_suffix(*this));
    if (operand)
        operand->print(out, child_prefix(prefix, is_last), true, "operand");
}

void VarNode::print(std::ostream &out, const std::string &prefix,
                    bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
}

void NumberNode::print(std::ostream &out, const std::string &prefix,
                       bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(value: " + std::to_string(value) + ")",
              annot_suffix(*this));
}

void RealNode::print(std::ostream &out, const std::string &prefix,
                     bool is_last, const std::string &role) const
{
    std::ostringstream v;
    v << value;
    emit_line(out, prefix, is_last, role,
              node_type() + "(value: " + v.str() + ")", annot_suffix(*this));
}

void CharNode::print(std::ostream &out, const std::string &prefix,
                     bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(value: '" + std::string(1, value) + "')",
              annot_suffix(*this));
}

void StringNode::print(std::ostream &out, const std::string &prefix,
                       bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(value: \"" + value + "\")", annot_suffix(*this));
}

void BoolNode::print(std::ostream &out, const std::string &prefix,
                     bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(value: " + (value ? "true" : "false") + ")",
              annot_suffix(*this));
}

void ArrayAccessNode::print(std::ostream &out, const std::string &prefix,
                            bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role, node_type(), annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    bool has_idx = !indices.empty();
    if (array) array->print(out, cp, !has_idx, "array");
    for (std::size_t i = 0; i < indices.size(); ++i)
        if (indices[i])
            indices[i]->print(out, cp, i + 1 == indices.size(), "index");
}

void RecordAccessNode::print(std::ostream &out, const std::string &prefix,
                             bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(field: '" + field_name + "')",
              annot_suffix(*this));
    if (record)
        record->print(out, child_prefix(prefix, is_last), true, "record");
}

void FuncCallExprNode::print(std::ostream &out, const std::string &prefix,
                             bool is_last, const std::string &role) const
{
    emit_line(out, prefix, is_last, role,
              node_type() + "(name: '" + name + "')", annot_suffix(*this));
    std::string cp = child_prefix(prefix, is_last);
    for (std::size_t i = 0; i < args.size(); ++i)
        if (args[i])
            args[i]->print(out, cp, i + 1 == args.size(), "arg");
}

// Entry Point

std::unique_ptr<ASTNode> ASTBuilder::build(const ParseTreeNode &root)
{
    if (is_node(root, "<program>"))
        return build_program(root);
    return build_statement(root);
}

// Program & Block

std::unique_ptr<ASTNode> ASTBuilder::build_program(const ParseTreeNode &n)
{
    std::unique_ptr<ProgramNode> program(new ProgramNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_node(c, "<program-header>"))
        {
            for (int j = 0; j < child_count(c); ++j)
                if (is_token(child(c, j), "ident"))
                    program->name = token_value(child(c, j));
        }
        else if (is_node(c, "<declaration-part>"))
        {
            std::unique_ptr<ASTNode> declarations = build_declaration_part(c);
            BlockNode *block = dynamic_cast<BlockNode *>(declarations.get());
            if (block)
            {
                for (std::size_t k = 0; k < block->statements.size(); ++k)
                    program->declarations.push_back(std::move(block->statements[k]));
            }
        }
        else if (is_node(c, "<compound-statement>"))
        {
            program->body = build_compound_statement(c);
        }
    }

    return std::unique_ptr<ASTNode>(std::move(program));
}

std::unique_ptr<ASTNode> ASTBuilder::build_block(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_node(c, "<declaration-part>"))
        {
            std::unique_ptr<ASTNode> declarations = build_declaration_part(c);
            BlockNode *decl_block = dynamic_cast<BlockNode *>(declarations.get());
            if (decl_block)
            {
                for (std::size_t j = 0; j < decl_block->statements.size(); ++j)
                    block->statements.push_back(std::move(decl_block->statements[j]));
            }
        }
        else if (is_node(c, "<compound-statement>"))
        {
            std::unique_ptr<ASTNode> statements = build_compound_statement(c);
            BlockNode *stmt_block = dynamic_cast<BlockNode *>(statements.get());
            if (stmt_block)
            {
                for (std::size_t j = 0; j < stmt_block->statements.size(); ++j)
                    block->statements.push_back(std::move(stmt_block->statements[j]));
            }
        }
    }

    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_compound_statement(const ParseTreeNode &n)
{
    for (int i = 0; i < child_count(n); ++i)
        if (is_node(child(n, i), "<statement-list>"))
            return build_statement_list(child(n, i));

    return std::unique_ptr<ASTNode>(new BlockNode());
}

std::unique_ptr<ASTNode> ASTBuilder::build_statement_list(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<statement>"))
        {
            std::unique_ptr<ASTNode> statement = build_statement(child(n, i));
            if (statement)
                block->statements.push_back(std::move(statement));
        }
    }

    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_statement(const ParseTreeNode &n)
{
    if (child_count(n) == 0)
        return std::unique_ptr<ASTNode>();

    const ParseTreeNode &c = child(n, 0);
    if (is_node(c, "<compound-statement>")) return build_compound_statement(c);
    if (is_node(c, "<assignment-statement>")) return build_assignment_statement(c);
    if (is_node(c, "<if-statement>")) return build_if_statement(c);
    if (is_node(c, "<while-statement>")) return build_while_statement(c);
    if (is_node(c, "<for-statement>")) return build_for_statement(c);
    if (is_node(c, "<repeat-statement>")) return build_repeat_statement(c);
    if (is_node(c, "<case-statement>")) return build_case_statement(c);
    if (is_node(c, "<procedure/function-call>")) return build_procedure_function_call(c);

    return std::unique_ptr<ASTNode>();
}

// Declarations

std::unique_ptr<ASTNode> ASTBuilder::build_declaration_part(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_node(c, "<const-declaration>"))
            append_from_container(block->statements, build_const_declaration(c));
        else if (is_node(c, "<type-declaration>"))
            append_from_container(block->statements, build_type_declaration(c));
        else if (is_node(c, "<var-declaration>"))
            append_from_container(block->statements, build_var_declaration(c));
        else if (is_node(c, "<subprogram-declaration>"))
            append_from_container(block->statements, build_subprogram_declaration(c));
    }

    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_const_declaration(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (!is_token(child(n, i), "ident"))
            continue;

        std::unique_ptr<ConstDeclNode> decl(new ConstDeclNode());
        decl->name = token_value(child(n, i));

        for (int j = i + 1; j < child_count(n); ++j)
        {
            if (is_node(child(n, j), "<constant>"))
            {
                decl->value = build_factor(child(n, j));
                i = j;
                break;
            }
        }

        block->statements.push_back(std::unique_ptr<ASTNode>(std::move(decl)));
    }

    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_type_declaration(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (!is_token(child(n, i), "ident"))
            continue;

        std::unique_ptr<TypeDeclNode> decl(new TypeDeclNode());
        decl->name = token_value(child(n, i));

        for (int j = i + 1; j < child_count(n); ++j)
        {
            if (is_node(child(n, j), "<type>"))
            {
                decl->type_def = build_type(child(n, j));
                i = j;
                break;
            }
        }

        block->statements.push_back(std::unique_ptr<ASTNode>(std::move(decl)));
    }

    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_var_declaration(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (!is_node(child(n, i), "<identifier-list>"))
            continue;

        std::unique_ptr<VarDeclNode> decl(new VarDeclNode());
        const ParseTreeNode &ids = child(n, i);
        for (int j = 0; j < child_count(ids); ++j)
            if (is_token(child(ids, j), "ident"))
                decl->names.push_back(token_value(child(ids, j)));

        for (int j = i + 1; j < child_count(n); ++j)
        {
            if (is_node(child(n, j), "<type>"))
            {
                decl->type_node = build_type(child(n, j));
                i = j;
                break;
            }
        }

        block->statements.push_back(std::unique_ptr<ASTNode>(std::move(decl)));
    }

    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_subprogram_declaration(const ParseTreeNode &n)
{
    if (child_count(n) == 0)
        return std::unique_ptr<ASTNode>();

    if (is_node(child(n, 0), "<procedure-declaration>"))
        return build_procedure_declaration(child(n, 0));
    if (is_node(child(n, 0), "<function-declaration>"))
        return build_function_declaration(child(n, 0));

    return std::unique_ptr<ASTNode>();
}

std::unique_ptr<ASTNode> ASTBuilder::build_procedure_declaration(const ParseTreeNode &n)
{
    std::unique_ptr<ProcDeclNode> decl(new ProcDeclNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "ident") && decl->name.empty())
            decl->name = token_value(c);
        else if (is_node(c, "<formal-parameter-list>"))
        {
            std::unique_ptr<ASTNode> params = build_formal_parameter_list(c);
            BlockNode *block = dynamic_cast<BlockNode *>(params.get());
            if (block)
                for (std::size_t j = 0; j < block->statements.size(); ++j)
                    decl->params.push_back(std::move(block->statements[j]));
        }
        else if (is_node(c, "<block>"))
            decl->body = build_block(c);
    }

    return std::unique_ptr<ASTNode>(std::move(decl));
}

std::unique_ptr<ASTNode> ASTBuilder::build_function_declaration(const ParseTreeNode &n)
{
    std::unique_ptr<FuncDeclNode> decl(new FuncDeclNode());
    bool seen_name = false;

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "ident") && !seen_name)
        {
            decl->name = token_value(c);
            seen_name = true;
        }
        else if (is_token(c, "ident") && !decl->return_type)
        {
            std::unique_ptr<VarNode> type(new VarNode());
            type->name = token_value(c);
            decl->return_type = std::unique_ptr<ASTNode>(std::move(type));
        }
        else if (is_node(c, "<formal-parameter-list>"))
        {
            std::unique_ptr<ASTNode> params = build_formal_parameter_list(c);
            BlockNode *block = dynamic_cast<BlockNode *>(params.get());
            if (block)
                for (std::size_t j = 0; j < block->statements.size(); ++j)
                    decl->params.push_back(std::move(block->statements[j]));
        }
        else if (is_node(c, "<block>"))
            decl->body = build_block(c);
    }

    return std::unique_ptr<ASTNode>(std::move(decl));
}

std::unique_ptr<ASTNode> ASTBuilder::build_formal_parameter_list(const ParseTreeNode &n)
{
    std::unique_ptr<BlockNode> block(new BlockNode());
    for (int i = 0; i < child_count(n); ++i)
        if (is_node(child(n, i), "<parameter-group>"))
            block->statements.push_back(build_parameter_group(child(n, i)));
    return std::unique_ptr<ASTNode>(std::move(block));
}

std::unique_ptr<ASTNode> ASTBuilder::build_parameter_group(const ParseTreeNode &n)
{
    std::unique_ptr<ParamGroupNode> param(new ParamGroupNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_node(c, "<identifier-list>"))
        {
            for (int j = 0; j < child_count(c); ++j)
                if (is_token(child(c, j), "ident"))
                    param->names.push_back(token_value(child(c, j)));
        }
        else if (is_node(c, "<array-type>"))
            param->type_node = build_array_type(c);
        else if (is_token(c, "ident"))
        {
            std::unique_ptr<VarNode> type(new VarNode());
            type->name = token_value(c);
            param->type_node = std::unique_ptr<ASTNode>(std::move(type));
        }
    }

    return std::unique_ptr<ASTNode>(std::move(param));
}

// Types

std::unique_ptr<ASTNode> ASTBuilder::build_type(const ParseTreeNode &n)
{
    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "ident"))
        {
            std::unique_ptr<VarNode> type(new VarNode());
            type->name = token_value(c);
            return std::unique_ptr<ASTNode>(std::move(type));
        }
        if (is_node(c, "<array-type>")) return build_array_type(c);
        if (is_node(c, "<record-type>")) return build_record_type(c);
        if (is_node(c, "<range>")) return build_range(c);
        if (is_node(c, "<enumerated>")) return build_enumerated(c);
    }

    return std::unique_ptr<ASTNode>();
}

std::unique_ptr<ASTNode> ASTBuilder::build_array_type(const ParseTreeNode &n)
{
    std::unique_ptr<ArrayTypeNode> type(new ArrayTypeNode());
    bool found_index = false;

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (!found_index && is_node(c, "<range>"))
        {
            type->index_range = build_range(c);
            found_index = true;
        }
        else if (!found_index && is_token(c, "ident"))
        {
            std::unique_ptr<VarNode> index(new VarNode());
            index->name = token_value(c);
            type->index_range = std::unique_ptr<ASTNode>(std::move(index));
            found_index = true;
        }
        else if (found_index && is_node(c, "<type>"))
            type->elem_type = build_type(c);
    }

    return std::unique_ptr<ASTNode>(std::move(type));
}

std::unique_ptr<ASTNode> ASTBuilder::build_record_type(const ParseTreeNode &n)
{
    std::unique_ptr<RecordTypeNode> record(new RecordTypeNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &field_list = child(n, i);
        if (!is_node(field_list, "<field-list>"))
            continue;

        for (int j = 0; j < child_count(field_list); ++j)
        {
            const ParseTreeNode &field_part = child(field_list, j);
            if (!is_node(field_part, "<field-part>"))
                continue;

            std::unique_ptr<VarDeclNode> field(new VarDeclNode());
            for (int k = 0; k < child_count(field_part); ++k)
            {
                const ParseTreeNode &c = child(field_part, k);
                if (is_node(c, "<identifier-list>"))
                    for (int m = 0; m < child_count(c); ++m)
                        if (is_token(child(c, m), "ident"))
                            field->names.push_back(token_value(child(c, m)));
                if (is_node(c, "<type>"))
                    field->type_node = build_type(c);
            }
            record->fields.push_back(std::unique_ptr<ASTNode>(std::move(field)));
        }
    }

    return std::unique_ptr<ASTNode>(std::move(record));
}

std::unique_ptr<ASTNode> ASTBuilder::build_range(const ParseTreeNode &n)
{
    std::unique_ptr<SubrangeNode> range(new SubrangeNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<constant>"))
        {
            if (!range->low)
                range->low = build_factor(child(n, i));
            else
                range->high = build_factor(child(n, i));
        }
    }

    return std::unique_ptr<ASTNode>(std::move(range));
}

std::unique_ptr<ASTNode> ASTBuilder::build_enumerated(const ParseTreeNode &n)
{
    std::unique_ptr<EnumNode> enum_node(new EnumNode());

    for (int i = 0; i < child_count(n); ++i)
        if (is_token(child(n, i), "ident"))
            enum_node->values.push_back(token_value(child(n, i)));

    return std::unique_ptr<ASTNode>(std::move(enum_node));
}

// Statements

std::unique_ptr<ASTNode> ASTBuilder::build_assignment_statement(const ParseTreeNode &n)
{
    std::unique_ptr<AssignNode> assign(new AssignNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<variable>"))
            assign->target = build_variable(child(n, i));
        else if (is_node(child(n, i), "<expression>"))
            assign->value = build_expression(child(n, i));
    }

    return std::unique_ptr<ASTNode>(std::move(assign));
}

std::unique_ptr<ASTNode> ASTBuilder::build_if_statement(const ParseTreeNode &n)
{
    std::unique_ptr<IfNode> stmt(new IfNode());
    int statement_count = 0;

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<expression>"))
            stmt->condition = build_expression(child(n, i));
        else if (is_node(child(n, i), "<statement>"))
        {
            if (statement_count == 0)
                stmt->then_branch = build_statement(child(n, i));
            else
                stmt->else_branch = build_statement(child(n, i));
            ++statement_count;
        }
    }

    return std::unique_ptr<ASTNode>(std::move(stmt));
}

std::unique_ptr<ASTNode> ASTBuilder::build_while_statement(const ParseTreeNode &n)
{
    std::unique_ptr<WhileNode> stmt(new WhileNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<expression>"))
            stmt->condition = build_expression(child(n, i));
        else if (is_node(child(n, i), "<statement>"))
            stmt->body = build_statement(child(n, i));
        else if (is_node(child(n, i), "<compound-statement>"))
            stmt->body = build_compound_statement(child(n, i));
    }

    return std::unique_ptr<ASTNode>(std::move(stmt));
}

std::unique_ptr<ASTNode> ASTBuilder::build_for_statement(const ParseTreeNode &n)
{
    std::unique_ptr<ForNode> stmt(new ForNode());
    int expr_count = 0;

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "ident"))
            stmt->var_name = token_value(c);
        else if (is_token(c, "downtosy"))
            stmt->is_downto = true;
        else if (is_node(c, "<expression>"))
        {
            if (expr_count == 0)
                stmt->from_expr = build_expression(c);
            else
                stmt->to_expr = build_expression(c);
            ++expr_count;
        }
        else if (is_node(c, "<statement>"))
            stmt->body = build_statement(c);
        else if (is_node(c, "<compound-statement>"))
            stmt->body = build_compound_statement(c);
    }

    return std::unique_ptr<ASTNode>(std::move(stmt));
}

std::unique_ptr<ASTNode> ASTBuilder::build_repeat_statement(const ParseTreeNode &n)
{
    std::unique_ptr<RepeatNode> stmt(new RepeatNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<statement-list>"))
        {
            std::unique_ptr<ASTNode> body = build_statement_list(child(n, i));
            BlockNode *block = dynamic_cast<BlockNode *>(body.get());
            if (block)
                for (std::size_t j = 0; j < block->statements.size(); ++j)
                    stmt->body.push_back(std::move(block->statements[j]));
        }
        else if (is_node(child(n, i), "<expression>"))
            stmt->condition = build_expression(child(n, i));
    }

    return std::unique_ptr<ASTNode>(std::move(stmt));
}

std::unique_ptr<ASTNode> ASTBuilder::build_case_statement(const ParseTreeNode &n)
{
    std::unique_ptr<CaseNode> stmt(new CaseNode());

    std::function<void(const ParseTreeNode &)> add_case_block = [&](const ParseTreeNode &block)
    {
        std::vector<std::unique_ptr<ASTNode> > labels;
        const ParseTreeNode *statement_node = nullptr;

        for (int i = 0; i < child_count(block); ++i)
        {
            const ParseTreeNode &c = child(block, i);
            if (is_node(c, "<constant>"))
                labels.push_back(build_factor(c));
            else if (is_node(c, "<statement>"))
                statement_node = &c;
            else if (is_node(c, "<case-block>"))
                add_case_block(c);
        }

        for (std::size_t i = 0; i < labels.size(); ++i)
        {
            std::unique_ptr<ASTNode> label = std::move(labels[i]);
            std::unique_ptr<ASTNode> statement_copy;
            if (statement_node)
                statement_copy = build_statement(*statement_node);
            stmt->cases.push_back(std::make_pair(std::move(label), std::move(statement_copy)));
        }
    };

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<expression>"))
            stmt->selector = build_expression(child(n, i));
        else if (is_node(child(n, i), "<case-block>"))
            add_case_block(child(n, i));
    }

    return std::unique_ptr<ASTNode>(std::move(stmt));
}

std::unique_ptr<ASTNode> ASTBuilder::build_procedure_function_call(const ParseTreeNode &n)
{
    std::unique_ptr<ProcCallNode> call(new ProcCallNode());

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "ident"))
            call->name = token_value(c);
        else if (is_node(c, "<parameter-list>"))
        {
            for (int j = 0; j < child_count(c); ++j)
                if (is_node(child(c, j), "<expression>"))
                    call->args.push_back(build_expression(child(c, j)));
        }
    }

    return std::unique_ptr<ASTNode>(std::move(call));
}

// Expressions

std::unique_ptr<ASTNode> ASTBuilder::build_expression(const ParseTreeNode &n)
{
    std::unique_ptr<ASTNode> left;

    for (int i = 0; i < child_count(n); ++i)
    {
        if (is_node(child(n, i), "<simple-expression>") && !left)
            left = build_simple_expression(child(n, i));
        else if (is_node(child(n, i), "<relational-operator>"))
        {
            std::unique_ptr<BinOpNode> op(new BinOpNode());
            op->op = operator_text(child(n, i));
            op->left = std::move(left);
            if (i + 1 < child_count(n))
                op->right = build_simple_expression(child(n, i + 1));
            left = std::unique_ptr<ASTNode>(std::move(op));
            break;
        }
    }

    return left;
}

std::unique_ptr<ASTNode> ASTBuilder::build_simple_expression(const ParseTreeNode &n)
{
    std::unique_ptr<ASTNode> left;
    std::string unary;

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if ((is_token(c, "plus") || is_token(c, "minus")) && !left)
            unary = operator_text(c);
        else if (is_node(c, "<term>") && !left)
        {
            left = build_term(c);
            if (unary == "-")
            {
                std::unique_ptr<UnaryOpNode> op(new UnaryOpNode());
                op->op = "-";
                op->operand = std::move(left);
                left = std::unique_ptr<ASTNode>(std::move(op));
            }
        }
        else if (is_node(c, "<additive-operator>"))
        {
            std::unique_ptr<BinOpNode> op(new BinOpNode());
            op->op = operator_text(c);
            op->left = std::move(left);
            if (i + 1 < child_count(n))
                op->right = build_term(child(n, i + 1));
            left = std::unique_ptr<ASTNode>(std::move(op));
        }
    }

    return left;
}

std::unique_ptr<ASTNode> ASTBuilder::build_term(const ParseTreeNode &n)
{
    std::unique_ptr<ASTNode> left;

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_node(c, "<factor>") && !left)
            left = build_factor(c);
        else if (is_node(c, "<multiplicative-operator>"))
        {
            std::unique_ptr<BinOpNode> op(new BinOpNode());
            op->op = operator_text(c);
            op->left = std::move(left);
            if (i + 1 < child_count(n))
                op->right = build_factor(child(n, i + 1));
            left = std::unique_ptr<ASTNode>(std::move(op));
        }
    }

    return left;
}

std::unique_ptr<ASTNode> ASTBuilder::build_factor(const ParseTreeNode &n)
{
    if (is_node(n, "<constant>"))
    {
        if (child_count(n) == 1)
            return make_literal_from_token(child(n, 0));

        if (child_count(n) >= 2 && is_token(child(n, 0), "minus"))
        {
            std::unique_ptr<UnaryOpNode> op(new UnaryOpNode());
            op->op = "-";
            op->operand = make_literal_from_token(child(n, 1));
            return std::unique_ptr<ASTNode>(std::move(op));
        }

        return make_literal_from_token(child(n, child_count(n) - 1));
    }

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "intcon") || is_token(c, "realcon") ||
            is_token(c, "charcon") || is_token(c, "string") ||
            is_token(c, "ident"))
            return make_literal_from_token(c);
        if (is_token(c, "notsy"))
        {
            std::unique_ptr<UnaryOpNode> op(new UnaryOpNode());
            op->op = "not";
            if (i + 1 < child_count(n))
                op->operand = build_factor(child(n, i + 1));
            return std::unique_ptr<ASTNode>(std::move(op));
        }
        if (is_node(c, "<expression>")) return build_expression(c);
        if (is_node(c, "<variable>")) return build_variable(c);
        if (is_node(c, "<procedure/function-call>"))
        {
            std::unique_ptr<FuncCallExprNode> call(new FuncCallExprNode());
            for (int j = 0; j < child_count(c); ++j)
            {
                if (is_token(child(c, j), "ident"))
                    call->name = token_value(child(c, j));
                else if (is_node(child(c, j), "<parameter-list>"))
                    for (int k = 0; k < child_count(child(c, j)); ++k)
                        if (is_node(child(child(c, j), k), "<expression>"))
                            call->args.push_back(build_expression(child(child(c, j), k)));
            }
            return std::unique_ptr<ASTNode>(std::move(call));
        }
    }

    return std::unique_ptr<ASTNode>();
}

std::unique_ptr<ASTNode> ASTBuilder::build_variable(const ParseTreeNode &n)
{
    std::unique_ptr<ASTNode> current;

    for (int i = 0; i < child_count(n); ++i)
    {
        const ParseTreeNode &c = child(n, i);
        if (is_token(c, "ident") && !current)
        {
            std::unique_ptr<VarNode> var(new VarNode());
            var->name = token_value(c);
            current = std::unique_ptr<ASTNode>(std::move(var));
        }
        else if (is_node(c, "<component-variable>"))
        {
            if (child_count(c) > 0 && is_token(child(c, 0), "period"))
            {
                std::unique_ptr<RecordAccessNode> access(new RecordAccessNode());
                access->record = std::move(current);
                for (int j = 0; j < child_count(c); ++j)
                    if (is_token(child(c, j), "ident"))
                        access->field_name = token_value(child(c, j));
                current = std::unique_ptr<ASTNode>(std::move(access));
            }
            else
            {
                std::unique_ptr<ArrayAccessNode> access(new ArrayAccessNode());
                access->array = std::move(current);

                for (int j = 0; j < child_count(c); ++j)
                {
                    if (!is_node(child(c, j), "<index-list>"))
                        continue;

                    std::function<void(const ParseTreeNode &)> add_indices = [&](const ParseTreeNode &idx)
                    {
                        for (int k = 0; k < child_count(idx); ++k)
                        {
                            const ParseTreeNode &item = child(idx, k);
                            if (is_token(item, "intcon") || is_token(item, "charcon") || is_token(item, "ident"))
                                access->indices.push_back(make_literal_from_token(item));
                            else if (is_node(item, "<index-list>"))
                                add_indices(item);
                        }
                    };

                    add_indices(child(c, j));
                }

                current = std::unique_ptr<ASTNode>(std::move(access));
            }
        }
    }

    return current;
}

// Helper

bool ASTBuilder::is_node(const ParseTreeNode &n, const std::string &name) const
{
    return n.name == name;
}

bool ASTBuilder::is_token(const ParseTreeNode &n, const std::string &token_type) const
{
    return token_has_type(n, token_type);
}

const ParseTreeNode &ASTBuilder::child(const ParseTreeNode &n, int i) const
{
    if (i < 0 || i >= child_count(n))
        throw std::out_of_range("ASTBuilder child index out of range");
    return *n.children[static_cast<std::size_t>(i)];
}

int ASTBuilder::child_count(const ParseTreeNode &n) const
{
    return static_cast<int>(n.children.size());
}

std::string ASTBuilder::token_value(const ParseTreeNode &n) const
{
    return token_text(n);
}
