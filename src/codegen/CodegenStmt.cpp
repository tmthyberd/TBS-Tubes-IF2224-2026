#include "CodegenVisitor.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace
{
std::string to_lower(const std::string &value)
{
    std::string result = value;
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

const SymbolTable &require_sym(const CodegenVisitor &visitor, const char *where)
{
    const SymbolTable *sym = visitor.symbol_table();
    if (!sym)
        throw std::runtime_error(std::string(where) + ": SymbolTable is not set");
    return *sym;
}



int variable_slot_size(const SymbolTable &sym, const TabEntry &entry)
{
    if (entry.type == TypeCode::ARRAY && entry.ref >= 0)
        return sym.get_atab(entry.ref).size;
    if (entry.type == TypeCode::RECORD && entry.ref >= 0)
        return sym.get_btab(entry.ref).vsze;
    return 1;
}

int main_frame_size(const SymbolTable &sym)
{
    int total = 0;
    int idx = sym.get_btab(0).last;
    while (idx >= 0)
    {
        const TabEntry &entry = sym.get_tab(idx);
        if (entry.obj == ObjClass::VARIABLE)
            total += variable_slot_size(sym, entry);
        idx = entry.link;
    }
    return VM_FRAME_HEADER_SIZE + total;
}

int resolve_var_tab_index(const SymbolTable &sym, int tab_index,
                          const std::string &name, const char *where)
{
    if (tab_index >= 0)
        return tab_index;

    int idx = sym.lookup(name);
    if (idx < 0)
        throw std::runtime_error(std::string(where) + ": unresolved variable '" +
                                 name + "'");
    return idx;
}
} 

std::vector<Instruction> CodegenVisitor::generate(ProgramNode &node)
{
    clear();
    visit_program(node);
    return instructions_;
}

void CodegenVisitor::visit_program(ProgramNode &node)
{
    const SymbolTable &sym = require_sym(*this, "visit_program");

    
    
    
    bool has_subprogram = false;
    for (auto &decl : node.declarations)
    {
        if (decl && (dynamic_cast<ProcDeclNode *>(decl.get()) ||
                     dynamic_cast<FuncDeclNode *>(decl.get())))
        {
            has_subprogram = true;
            break;
        }
    }

    int jump_over_subprograms = -1;
    if (has_subprogram)
    {
        jump_over_subprograms = emit(Instruction::make(OpCode::JMP, 0, 0));
        for (auto &decl : node.declarations)
        {
            if (decl && (dynamic_cast<ProcDeclNode *>(decl.get()) ||
                         dynamic_cast<FuncDeclNode *>(decl.get())))
                visit(*decl);
        }
        patch_operand(jump_over_subprograms, current_instruction_index());
    }

    
    emit(Instruction::make(OpCode::INT, 0, main_frame_size(sym)));

    if (node.body)
        visit(*node.body);

    emit(Instruction::make(OpCode::RET, 0, 0));
}

void CodegenVisitor::visit_block(BlockNode &node)
{
    for (auto &stmt : node.statements)
        if (stmt)
            visit(*stmt);
}

void CodegenVisitor::visit_assign(AssignNode &node)
{
    const SymbolTable &sym = require_sym(*this, "visit_assign");

    if (!node.target)
        throw std::runtime_error("visit_assign: missing assignment target");
    if (!node.value)
        throw std::runtime_error("visit_assign: missing assignment value");


    if (auto *var = dynamic_cast<VarNode *>(node.target.get()))
    {
        if (current_func_tab_index_ >= 0 &&
            var->tab_index == current_func_tab_index_)
        {
            visit(*node.value);
            emit(Instruction::make(OpCode::STO, current_func_level_,
                                   current_func_result_addr_));
            return;
        }

        int idx = resolve_var_tab_index(sym, var->tab_index, var->name,
                                        "visit_assign");
        const TabEntry &entry = sym.get_tab(idx);

        visit(*node.value);
        emit(Instruction::make(OpCode::STO, entry.lev,
                               entry.adr + VM_FRAME_HEADER_SIZE));
        return;
    }
   
    emit_lvalue_address(*node.target);
    visit(*node.value);
    emit(Instruction::make(OpCode::STI, 0, 0));
}

void CodegenVisitor::visit_if(IfNode &node)
{
    if (!node.condition)
        throw std::runtime_error("visit_if: missing condition");

    visit(*node.condition);
    int jpc_to_else = emit(Instruction::make(OpCode::JPC, 0, 0));

    if (node.then_branch)
        visit(*node.then_branch);

    if (node.else_branch)
    {
        int jmp_to_end = emit(Instruction::make(OpCode::JMP, 0, 0));
        patch_operand(jpc_to_else, current_instruction_index());
        visit(*node.else_branch);
        patch_operand(jmp_to_end, current_instruction_index());
    }
    else
    {
        patch_operand(jpc_to_else, current_instruction_index());
    }
}

void CodegenVisitor::visit_while(WhileNode &node)
{
    if (!node.condition)
        throw std::runtime_error("visit_while: missing condition");

    int loop_start = current_instruction_index();
    visit(*node.condition);
    int jpc_to_end = emit(Instruction::make(OpCode::JPC, 0, 0));

    if (node.body)
        visit(*node.body);

    emit(Instruction::make(OpCode::JMP, 0, loop_start));
    patch_operand(jpc_to_end, current_instruction_index());
}

void CodegenVisitor::visit_for(ForNode &node)
{
    const SymbolTable &sym = require_sym(*this, "visit_for");

    int idx = resolve_var_tab_index(sym, node.tab_index, node.var_name,
                                    "visit_for");
    const TabEntry &entry = sym.get_tab(idx);
    int var_level = entry.lev;
    int var_addr = entry.adr + VM_FRAME_HEADER_SIZE;

    
    if (!node.from_expr || !node.to_expr)
        throw std::runtime_error("visit_for: missing loop bounds");
    visit(*node.from_expr);
    emit(Instruction::make(OpCode::STO, var_level, var_addr));

    
    int cond_start = current_instruction_index();
    emit(Instruction::make(OpCode::LOD, var_level, var_addr));
    visit(*node.to_expr);
    emit(Instruction::operation(node.is_downto ? OprCode::GEQ : OprCode::LEQ));
    int jpc_to_end = emit(Instruction::make(OpCode::JPC, 0, 0));

    
    if (node.body)
        visit(*node.body);

    
    emit(Instruction::make(OpCode::LOD, var_level, var_addr));
    emit(Instruction::literal(VMValue::integer(1)));
    emit(Instruction::operation(node.is_downto ? OprCode::SUB : OprCode::ADD));
    emit(Instruction::make(OpCode::STO, var_level, var_addr));

    emit(Instruction::make(OpCode::JMP, 0, cond_start));
    patch_operand(jpc_to_end, current_instruction_index());
}

void CodegenVisitor::visit_repeat(RepeatNode &node)
{
    int loop_start = current_instruction_index();

    for (auto &stmt : node.body)
        if (stmt)
            visit(*stmt);

    if (!node.condition)
        throw std::runtime_error("visit_repeat: missing condition");

    
    visit(*node.condition);
    emit(Instruction::make(OpCode::JPC, 0, loop_start));
}

void CodegenVisitor::visit_case(CaseNode &node)
{
    if (!node.selector)
        throw std::runtime_error("visit_case: missing selector");

    std::vector<int> jumps_to_end;
    const std::size_t n_cases = node.cases.size();

    for (std::size_t i = 0; i < n_cases; ++i)
    {
        ASTNode *label = node.cases[i].first.get();
        ASTNode *stmt = node.cases[i].second.get();
      
        visit(*node.selector);
        if (label)
            visit(*label);
        emit(Instruction::operation(OprCode::EQL));
        int jpc_next = emit(Instruction::make(OpCode::JPC, 0, 0));

        if (stmt)
            visit(*stmt);
        
        if (i + 1 < n_cases)
            jumps_to_end.push_back(emit(Instruction::make(OpCode::JMP, 0, 0)));

        patch_operand(jpc_next, current_instruction_index());
    }

    int case_end = current_instruction_index();
    for (int jmp_idx : jumps_to_end)
        patch_operand(jmp_idx, case_end);
}

void CodegenVisitor::visit_proc_call(ProcCallNode &node)
{
    const SymbolTable &sym = require_sym(*this, "visit_proc_call");
    const std::string name = to_lower(node.name);

    
    if (name == "writeln")
    {
        if (node.args.empty())
        {
            emit(Instruction::operation(OprCode::WRTLN));
            return;
        }
        for (std::size_t i = 0; i < node.args.size(); ++i)
        {
            if (node.args[i])
                visit(*node.args[i]);
            bool last = (i + 1 == node.args.size());
            emit(Instruction::operation(last ? OprCode::WRTLN : OprCode::WRT));
        }
        return;
    }

    if (name == "write")
    {
        for (auto &arg : node.args)
        {
            if (arg)
                visit(*arg);
            emit(Instruction::operation(OprCode::WRT));
        }
        return;
    }

    
    int idx = node.tab_index >= 0 ? node.tab_index : sym.lookup(node.name);
    if (idx < 0)
        throw std::runtime_error("visit_proc_call: unresolved procedure '" +
                                 node.name + "'");

    const TabEntry &entry = sym.get_tab(idx);
    for (auto &arg : node.args)
        if (arg)
            visit(*arg);

    emit(Instruction::make(OpCode::CAL, entry.lev, entry.adr));
}

void CodegenVisitor::codegen_subprogram(const std::string &name, ASTNode *body,
                                        bool is_function)
{
    const SymbolTable &sym = require_sym(*this, "codegen_subprogram");

    int idx = sym.lookup(name);
    if (idx >= 0)
        const_cast<SymbolTable &>(sym).get_tab(idx).adr = current_instruction_index();

    int psze = 0, vsze = 0, decl_level = 0;
    if (idx >= 0)
    {
        decl_level = sym.get_tab(idx).lev;
        int btab_idx = sym.get_tab(idx).ref;
        if (btab_idx >= 0)
        {
            const BtabEntry &block = sym.get_btab(btab_idx);
            psze = block.psze;
            vsze = block.vsze;
        }
    }

    int body_level = decl_level + 1;

    int frame = VM_FRAME_HEADER_SIZE + psze + vsze;
    int result_addr = -1;
    if (is_function)
    {
        result_addr = VM_FRAME_HEADER_SIZE + psze + vsze;
        frame += 1;
    }

    emit(Instruction::make(OpCode::INT, 0, frame));

    for (int i = psze - 1; i >= 0; --i)
        emit(Instruction::make(OpCode::STO, body_level,
                               VM_FRAME_HEADER_SIZE + i));

    int saved_idx = current_func_tab_index_;
    int saved_addr = current_func_result_addr_;
    int saved_level = current_func_level_;

    current_func_tab_index_ = is_function ? idx : -1;
    current_func_result_addr_ = result_addr;
    current_func_level_ = body_level;

    if (body)
        visit(*body);

    current_func_tab_index_ = saved_idx;
    current_func_result_addr_ = saved_addr;
    current_func_level_ = saved_level;

    if (is_function)
        emit(Instruction::make(OpCode::LOD, body_level, result_addr));

    emit(Instruction::make(OpCode::RET, 0, 0));
}

void CodegenVisitor::visit_proc_decl(ProcDeclNode &node)
{
    codegen_subprogram(node.name, node.body.get(), false);
}

void CodegenVisitor::visit_func_decl(FuncDeclNode &node)
{
    codegen_subprogram(node.name, node.body.get(), true);
}

void CodegenVisitor::visit_var_decl(VarDeclNode &) {}
void CodegenVisitor::visit_const_decl(ConstDeclNode &) {}
void CodegenVisitor::visit_type_decl(TypeDeclNode &) {}
void CodegenVisitor::visit_param_group(ParamGroupNode &) {}
void CodegenVisitor::visit_array_type(ArrayTypeNode &) {}
void CodegenVisitor::visit_record_type(RecordTypeNode &) {}
void CodegenVisitor::visit_subrange(SubrangeNode &) {}
void CodegenVisitor::visit_enum(EnumNode &) {}
