#ifndef CODEGEN_VISITOR_HPP
#define CODEGEN_VISITOR_HPP

#include <stdexcept>
#include <vector>

#include "Instruction.hpp"
#include "../interpreter/VM_Memory.hpp"
#include "../semantic/ast/ASTNode.hpp"
#include "../semantic/ast/DeclNodes.hpp"
#include "../semantic/ast/ExprNodes.hpp"
#include "../semantic/ast/StmtNodes.hpp"
#include "../semantic/symbol_table/Symbol_Table.hpp"

class CodegenVisitor
{
public:
    CodegenVisitor() : sym_(nullptr) {}
    explicit CodegenVisitor(const SymbolTable &sym) : sym_(&sym) {}

    std::vector<Instruction> generate(ProgramNode &node);
    void visit(ASTNode &node);

    void visit_program(ProgramNode &node);
    void visit_var_decl(VarDeclNode &node);
    void visit_const_decl(ConstDeclNode &node);
    void visit_type_decl(TypeDeclNode &node);
    void visit_proc_decl(ProcDeclNode &node);
    void visit_func_decl(FuncDeclNode &node);
    void visit_param_group(ParamGroupNode &node);
    void visit_array_type(ArrayTypeNode &node);
    void visit_record_type(RecordTypeNode &node);
    void visit_subrange(SubrangeNode &node);
    void visit_enum(EnumNode &node);

    void visit_block(BlockNode &node);
    void visit_assign(AssignNode &node);
    void visit_if(IfNode &node);
    void visit_while(WhileNode &node);
    void visit_for(ForNode &node);
    void visit_repeat(RepeatNode &node);
    void visit_case(CaseNode &node);
    void visit_proc_call(ProcCallNode &node);

    void visit_binop(BinOpNode &node);
    void visit_unary(UnaryOpNode &node);
    void visit_var(VarNode &node);
    void visit_number(NumberNode &node);
    void visit_real(RealNode &node);
    void visit_char(CharNode &node);
    void visit_string(StringNode &node);
    void visit_bool(BoolNode &node);
    void visit_array_access(ArrayAccessNode &node);
    void visit_record_access(RecordAccessNode &node);
    void visit_func_call_expr(FuncCallExprNode &node);

    int emit(const Instruction &instruction)
    {
        instructions_.push_back(instruction);
        return static_cast<int>(instructions_.size()) - 1;
    }

    void patch_operand(int instruction_index, int operand)
    {
        if (instruction_index < 0 ||
            instruction_index >= static_cast<int>(instructions_.size()))
            throw std::out_of_range("CodegenVisitor patch index out of range");
        instructions_[static_cast<std::size_t>(instruction_index)].operand = operand;
    }

    int current_instruction_index() const
    {
        return static_cast<int>(instructions_.size());
    }

    const std::vector<Instruction> &instructions() const
    {
        return instructions_;
    }

    std::vector<Instruction> &mutable_instructions()
    {
        return instructions_;
    }

    void clear()
    {
        instructions_.clear();
    }

    void set_symbol_table(const SymbolTable &sym)
    {
        sym_ = &sym;
    }

    const SymbolTable *symbol_table() const
    {
        return sym_;
    }

    int variable_address_from_tab_index(int tab_index) const
    {
        if (!sym_)
            throw std::runtime_error("CodegenVisitor requires SymbolTable for variable addresses");
        if (tab_index < 0)
            throw std::runtime_error("Cannot resolve variable address without tab index");
        return sym_->get_tab(tab_index).adr + VM_FRAME_HEADER_SIZE;
    }

protected:
    std::vector<Instruction> instructions_;
    const SymbolTable *sym_;
};

#endif
