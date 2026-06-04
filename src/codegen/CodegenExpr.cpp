#include "CodegenVisitor.hpp"

#include <stdexcept>
#include <string>

namespace
{
const SymbolTable &require_symbol_table(const CodegenVisitor &visitor,
                                        const char *where)
{
    const SymbolTable *sym = visitor.symbol_table();
    if (!sym)
        throw std::runtime_error(std::string(where) + ": SymbolTable is not set");
    return *sym;
}

int lvalue_ref(const CodegenVisitor &visitor, const ASTNode &node)
{
    const SymbolTable &sym = require_symbol_table(visitor, "lvalue_ref");
    if (node.tab_index < 0)
        return -1;
    return sym.get_tab(node.tab_index).ref;
}

void emit_lvalue_address_impl(CodegenVisitor &visitor, ASTNode &node)
{
    const SymbolTable &sym = require_symbol_table(visitor, "emit_lvalue_address");

    if (auto *var = dynamic_cast<VarNode *>(&node))
    {
        if (var->tab_index < 0)
            throw std::runtime_error("emit_lvalue_address: unresolved variable '" +
                                     var->name + "'");

        const TabEntry &entry = sym.get_tab(var->tab_index);
        if (entry.obj != ObjClass::VARIABLE)
            throw std::runtime_error("emit_lvalue_address: '" + var->name +
                                     "' is not an assignable variable");

        visitor.emit(Instruction::make(OpCode::LDA,
                                       entry.lev,
                                       entry.adr + VM_FRAME_HEADER_SIZE));
        return;
    }

    if (auto *access = dynamic_cast<ArrayAccessNode *>(&node))
    {
        if (access->indices.empty())
            throw std::runtime_error("emit_lvalue_address: array access has no indices");
        if (access->indices.size() != 1)
            throw std::runtime_error("emit_lvalue_address: multidimensional arrays "
                                     "need an explicit VM addressing convention");

        int atab_idx = lvalue_ref(visitor, *access->array);
        if (atab_idx < 0)
            throw std::runtime_error("emit_lvalue_address: unresolved array type metadata");

        const AtabEntry &atab = sym.get_atab(atab_idx);

        emit_lvalue_address_impl(visitor, *access->array);
        visitor.visit(*access->indices[0]);

        if (atab.low != 0)
        {
            visitor.emit(Instruction::literal(VMValue::integer(atab.low)));
            visitor.emit(Instruction::operation(OprCode::SUB));
        }

        if (atab.elsz != 1)
        {
            visitor.emit(Instruction::literal(VMValue::integer(atab.elsz)));
            visitor.emit(Instruction::operation(OprCode::MUL));
        }

        visitor.emit(Instruction::operation(OprCode::ADD));
        return;
    }

    if (auto *access = dynamic_cast<RecordAccessNode *>(&node))
    {
        if (access->tab_index < 0)
            throw std::runtime_error("emit_lvalue_address: unresolved record field '" +
                                     access->field_name + "'");

        const TabEntry &field = sym.get_tab(access->tab_index);

        emit_lvalue_address_impl(visitor, *access->record);
        if (field.adr != 0)
        {
            visitor.emit(Instruction::literal(VMValue::integer(field.adr)));
            visitor.emit(Instruction::operation(OprCode::ADD));
        }
        return;
    }

    throw std::runtime_error("emit_lvalue_address: node is not an lvalue");
}
}

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
    const SymbolTable &sym = require_symbol_table(*this, "visit_var");

    if (node.tab_index < 0)
        throw std::runtime_error("visit_var: unresolved variable '" + node.name + "'");

    const TabEntry &entry = sym.get_tab(node.tab_index);
    if (entry.obj != ObjClass::VARIABLE)
        throw std::runtime_error("visit_var: '" + node.name +
                                 "' is not a runtime variable");

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
        emit(Instruction::operation(OprCode::AND));
        return;
    }

    if (op == "or")
    {
        visit(*node.left);
        visit(*node.right);
        emit(Instruction::operation(OprCode::OR));
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
    if (op == "div") return OprCode::IDIV;
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

void CodegenVisitor::emit_lvalue_address(ASTNode &node)
{
    emit_lvalue_address_impl(*this, node);
}

void CodegenVisitor::visit_array_access(ArrayAccessNode &node)
{
    emit_lvalue_address(node);
    emit(Instruction::make(OpCode::LDI));
}

void CodegenVisitor::visit_record_access(RecordAccessNode &node)
{
    emit_lvalue_address(node);
    emit(Instruction::make(OpCode::LDI));
}

void CodegenVisitor::visit_func_call_expr(FuncCallExprNode &node)
{
    const SymbolTable &sym = require_symbol_table(*this, "visit_func_call_expr");

    int func_tab_idx = node.tab_index;
    if (func_tab_idx < 0)
        throw std::runtime_error("visit_func_call_expr: unresolved function '" +
                                  node.name + "'");

    const TabEntry &func_entry = sym.get_tab(func_tab_idx);
    int func_lev = func_entry.lev;
    int func_adr = func_entry.adr;

    for (auto &arg : node.args)
        visit(*arg);

    emit(Instruction::make(OpCode::CAL, func_lev, func_adr));
}
