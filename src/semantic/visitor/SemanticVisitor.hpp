#ifndef SEMANTIC_VISITOR_HPP
#define SEMANTIC_VISITOR_HPP

#include "../ast/ASTNode.hpp"
#include "../ast/ExprNodes.hpp"
#include "../ast/StmtNodes.hpp"
#include "../ast/DeclNodes.hpp"
#include "../symbol_table/Symbol_Table.hpp"
#include "../error/SemanticError.hpp"

class SemanticVisitor
{
public:
    explicit SemanticVisitor(SymbolTable &sym);

    void visit(ASTNode &n);

    void visit_program(ProgramNode &n);
    void visit_var_decl(VarDeclNode &n);
    void visit_const_decl(ConstDeclNode &n);
    void visit_type_decl(TypeDeclNode &n);
    void visit_proc_decl(ProcDeclNode &n);
    void visit_func_decl(FuncDeclNode &n);
    void visit_param_group(ParamGroupNode &n);
    void visit_array_type(ArrayTypeNode &n);
    void visit_record_type(RecordTypeNode &n);
    void visit_subrange(SubrangeNode &n);
    void visit_enum(EnumNode &n);

    void visit_block(BlockNode &n);
    void visit_assign(AssignNode &n);
    void visit_if(IfNode &n);
    void visit_while(WhileNode &n);
    void visit_for(ForNode &n);
    void visit_repeat(RepeatNode &n);
    void visit_case(CaseNode &n);
    void visit_proc_call(ProcCallNode &n);
    void visit_binop(BinOpNode &n);
    void visit_unary(UnaryOpNode &n);
    void visit_var(VarNode &n);
    void visit_number(NumberNode &n);
    void visit_real(RealNode &n);
    void visit_char(CharNode &n);
    void visit_string(StringNode &n);
    void visit_bool(BoolNode &n);
    void visit_array_access(ArrayAccessNode &n);
    void visit_record_access(RecordAccessNode &n);
    void visit_func_call_expr(FuncCallExprNode &n);

protected:
    SymbolTable &sym;

    bool is_compatible(TypeCode t1, TypeCode t2) const;

    bool is_assignment_compatible(TypeCode t1, TypeCode t2) const;

    TypeCode resolve_binop_type(const std::string &op,
                                TypeCode left_type,
                                TypeCode right_type) const;

    void type_error(const std::string &msg, int line) const;
    void undeclared_error(const std::string &name, int line) const;
    void redeclared_error(const std::string &name, int line) const;

    bool inside_loop = false;
};

#endif