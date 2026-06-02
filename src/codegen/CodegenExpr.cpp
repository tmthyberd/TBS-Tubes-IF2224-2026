#include "CodegenVisitor.hpp"

#include <stdexcept>
#include <string>

void CodegenVisitor::visit(ASTNode &node)
{
    if (auto *p = dynamic_cast<NumberNode *>(&node))        { visit_number(*p);       return; }
    if (auto *p = dynamic_cast<RealNode *>(&node))          { visit_real(*p);         return; }
    if (auto *p = dynamic_cast<CharNode *>(&node))          { visit_char(*p);         return; }
    if (auto *p = dynamic_cast<StringNode *>(&node))        { visit_string(*p);       return; }
    if (auto *p = dynamic_cast<BoolNode *>(&node))          { visit_bool(*p);         return; }
    if (auto *p = dynamic_cast<VarNode *>(&node))           { visit_var(*p);          return; }
    if (auto *p = dynamic_cast<BinOpNode *>(&node))         { visit_binop(*p);        return; }
    if (auto *p = dynamic_cast<UnaryOpNode *>(&node))       { visit_unary(*p);        return; }
    if (auto *p = dynamic_cast<ArrayAccessNode *>(&node))   { visit_array_access(*p); return; }
    if (auto *p = dynamic_cast<RecordAccessNode *>(&node))  { visit_record_access(*p);return; }
    if (auto *p = dynamic_cast<FuncCallExprNode *>(&node))  { visit_func_call_expr(*p);return; }

    if (auto *p = dynamic_cast<ProgramNode *>(&node))       { visit_program(*p);      return; }
    if (auto *p = dynamic_cast<BlockNode *>(&node))         { visit_block(*p);        return; }
    if (auto *p = dynamic_cast<AssignNode *>(&node))        { visit_assign(*p);       return; }
    if (auto *p = dynamic_cast<IfNode *>(&node))            { visit_if(*p);           return; }
    if (auto *p = dynamic_cast<WhileNode *>(&node))         { visit_while(*p);        return; }
    if (auto *p = dynamic_cast<ForNode *>(&node))           { visit_for(*p);          return; }
    if (auto *p = dynamic_cast<RepeatNode *>(&node))        { visit_repeat(*p);       return; }
    if (auto *p = dynamic_cast<CaseNode *>(&node))          { visit_case(*p);         return; }
    if (auto *p = dynamic_cast<ProcCallNode *>(&node))      { visit_proc_call(*p);    return; }

    if (dynamic_cast<VarDeclNode *>(&node))    return;
    if (dynamic_cast<ConstDeclNode *>(&node))  return;
    if (dynamic_cast<TypeDeclNode *>(&node))   return;
    if (dynamic_cast<ParamGroupNode *>(&node)) return;
    if (dynamic_cast<ArrayTypeNode *>(&node))  return;
    if (dynamic_cast<RecordTypeNode *>(&node)) return;
    if (dynamic_cast<SubrangeNode *>(&node))   return;
    if (dynamic_cast<EnumNode *>(&node))       return;

    if (auto *p = dynamic_cast<ProcDeclNode *>(&node)) { visit_proc_decl(*p); return; }
    if (auto *p = dynamic_cast<FuncDeclNode *>(&node)) { visit_func_decl(*p); return; }

    throw std::runtime_error("CodegenVisitor::visit: unknown AST node type");
}

void CodegenVisitor::visit_number(NumberNode &node)
{
    emit(Instruction::literal(VMValue::integer(node.value)));
}

void CodegenVisitor::visit_real(RealNode &node)
{
    emit(Instruction::literal(VMValue::real(node.value)));
}

void CodegenVisitor::visit_char(CharNode &node)
{
    emit(Instruction::literal(VMValue::character(node.value)));
}

void CodegenVisitor::visit_string(StringNode &node)
{
    emit(Instruction::literal(VMValue::string(node.value)));
}

void CodegenVisitor::visit_bool(BoolNode &node)
{
    emit(Instruction::literal(VMValue::boolean(node.value)));
}

void CodegenVisitor::visit_var(VarNode &node)
{
    if (!sym_)
        throw std::runtime_error("visit_var: SymbolTable is not set");

    if (node.tab_index < 0)
        throw std::runtime_error("visit_var: unresolved variable '" + node.name + "'");

    const TabEntry &entry = sym_->get_tab(node.tab_index);
    int address = variable_address_from_tab_index(node.tab_index);
    int level   = entry.lev;

    emit(Instruction::make(OpCode::LOD, level, address));
}

void CodegenVisitor::visit_binop(BinOpNode &node)
{
    const std::string &op = node.op;

    if (op == "and")
    {
        visit(*node.left);
        visit(*node.right);
        emit(Instruction::operation(OprCode::MUL));
        emit(Instruction::literal(VMValue::integer(0)));
        emit(Instruction::operation(OprCode::NEQ));
        return;
    }

    if (op == "or")
    {
        visit(*node.left);
        visit(*node.right);
        emit(Instruction::operation(OprCode::ADD));
        emit(Instruction::literal(VMValue::integer(0)));
        emit(Instruction::operation(OprCode::GTR));
        return;
    }

    visit(*node.left);
    visit(*node.right);

    OprCode opr = ast_op_to_opr(op);
    emit(Instruction::operation(opr));
}

OprCode CodegenVisitor::ast_op_to_opr(const std::string &op)
{
    if (op == "+")   return OprCode::ADD;
    if (op == "-")   return OprCode::SUB;
    if (op == "*")   return OprCode::MUL;
    if (op == "/")   return OprCode::DIV;
    if (op == "div") return OprCode::DIV;
    if (op == "mod") return OprCode::MOD;

    if (op == "==")  return OprCode::EQL;
    if (op == "<>")  return OprCode::NEQ;
    if (op == "<")   return OprCode::LSS;
    if (op == ">=")  return OprCode::GEQ;
    if (op == ">")   return OprCode::GTR;
    if (op == "<=")  return OprCode::LEQ;

    throw std::runtime_error("ast_op_to_opr: unknown operator '" + op + "'");
}

void CodegenVisitor::visit_unary(UnaryOpNode &node)
{
    visit(*node.operand);

    if (node.op == "-")
    {
        emit(Instruction::operation(OprCode::NEG));
    }
    else if (node.op == "not")
    {
        emit(Instruction::literal(VMValue::boolean(false)));
        emit(Instruction::operation(OprCode::EQL));
    }
    else
    {
        throw std::runtime_error("visit_unary: unknown operator '" + node.op + "'");
    }
}

void CodegenVisitor::visit_array_access(ArrayAccessNode &node)
{
    if (!sym_)
        throw std::runtime_error("visit_array_access: SymbolTable is not set");

    int arr_tab_idx = node.array->tab_index;
    if (arr_tab_idx < 0)
        throw std::runtime_error("visit_array_access: unresolved array variable");

    const TabEntry &arr_entry = sym_->get_tab(arr_tab_idx);
    int base_adr = arr_entry.adr + VM_FRAME_HEADER_SIZE;
    int level    = arr_entry.lev;

    int low = 0;
    if (arr_entry.ref >= 0)
    {
        const AtabEntry &atab = sym_->get_atab(arr_entry.ref);
        low = atab.low;
    }

    if (node.indices.empty())
        throw std::runtime_error("visit_array_access: array access has no indices");

    visit(*node.indices[0]);

    if (low != 0)
    {
        emit(Instruction::literal(VMValue::integer(low)));
        emit(Instruction::operation(OprCode::SUB));
    }

    emit(Instruction::literal(VMValue::integer(base_adr)));
    emit(Instruction::operation(OprCode::ADD));

    emit(Instruction::make(OpCode::LOD, level, base_adr));
}

void CodegenVisitor::visit_record_access(RecordAccessNode &node)
{
    if (!sym_)
        throw std::runtime_error("visit_record_access: SymbolTable is not set");

    int field_tab_idx = node.tab_index;
    if (field_tab_idx < 0)
        throw std::runtime_error("visit_record_access: unresolved field '" +
                                  node.field_name + "'");

    const TabEntry &field_entry = sym_->get_tab(field_tab_idx);
    int address = field_entry.adr + VM_FRAME_HEADER_SIZE;
    int level   = field_entry.lev;

    emit(Instruction::make(OpCode::LOD, level, address));
}

void CodegenVisitor::visit_func_call_expr(FuncCallExprNode &node)
{
    if (!sym_)
        throw std::runtime_error("visit_func_call_expr: SymbolTable is not set");

    int func_tab_idx = node.tab_index;
    if (func_tab_idx < 0)
        throw std::runtime_error("visit_func_call_expr: unresolved function '" +
                                  node.name + "'");

    const TabEntry &func_entry = sym_->get_tab(func_tab_idx);
    int func_lev = func_entry.lev;
    int func_adr = func_entry.adr;

    for (auto &arg : node.args)
        visit(*arg);

    emit(Instruction::make(OpCode::CAL, func_lev, func_adr));
}