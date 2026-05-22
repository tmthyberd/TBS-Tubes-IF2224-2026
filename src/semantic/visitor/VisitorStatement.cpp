#include "SemanticVisitor.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

static std::string typecode_to_string(TypeCode t)
{
    switch (t)
    {
    case TypeCode::INTEGER:  return "Integer";
    case TypeCode::REAL:     return "Real";
    case TypeCode::CHAR:     return "Char";
    case TypeCode::BOOLEAN:  return "Boolean";
    case TypeCode::STRING:   return "String";
    case TypeCode::ARRAY:    return "Array";
    case TypeCode::RECORD:   return "Record";
    case TypeCode::SUBRANGE: return "Subrange";
    case TypeCode::ENUM:     return "Enum";
    case TypeCode::VOID:     return "Void";
    default:                 return "Unknown";
    }
}

SemanticVisitor::SemanticVisitor(SymbolTable &sym) : sym(sym) {}

void SemanticVisitor::visit(ASTNode &n)
{
    if (auto *p = dynamic_cast<ProgramNode *>(&n))        visit_program(*p);
    else if (auto *p = dynamic_cast<VarDeclNode *>(&n))   visit_var_decl(*p);
    else if (auto *p = dynamic_cast<ConstDeclNode *>(&n)) visit_const_decl(*p);
    else if (auto *p = dynamic_cast<TypeDeclNode *>(&n))  visit_type_decl(*p);
    else if (auto *p = dynamic_cast<ProcDeclNode *>(&n))  visit_proc_decl(*p);
    else if (auto *p = dynamic_cast<FuncDeclNode *>(&n))  visit_func_decl(*p);
    else if (auto *p = dynamic_cast<ParamGroupNode *>(&n))visit_param_group(*p);
    else if (auto *p = dynamic_cast<ArrayTypeNode *>(&n)) visit_array_type(*p);
    else if (auto *p = dynamic_cast<RecordTypeNode *>(&n))visit_record_type(*p);
    else if (auto *p = dynamic_cast<SubrangeNode *>(&n))  visit_subrange(*p);
    else if (auto *p = dynamic_cast<EnumNode *>(&n))      visit_enum(*p);
    else if (auto *p = dynamic_cast<BlockNode *>(&n))     visit_block(*p);
    else if (auto *p = dynamic_cast<AssignNode *>(&n))    visit_assign(*p);
    else if (auto *p = dynamic_cast<IfNode *>(&n))        visit_if(*p);
    else if (auto *p = dynamic_cast<WhileNode *>(&n))     visit_while(*p);
    else if (auto *p = dynamic_cast<ForNode *>(&n))       visit_for(*p);
    else if (auto *p = dynamic_cast<RepeatNode *>(&n))    visit_repeat(*p);
    else if (auto *p = dynamic_cast<CaseNode *>(&n))      visit_case(*p);
    else if (auto *p = dynamic_cast<ProcCallNode *>(&n))  visit_proc_call(*p);
    else if (auto *p = dynamic_cast<BinOpNode *>(&n))     visit_binop(*p);
    else if (auto *p = dynamic_cast<UnaryOpNode *>(&n))   visit_unary(*p);
    else if (auto *p = dynamic_cast<VarNode *>(&n))       visit_var(*p);
    else if (auto *p = dynamic_cast<NumberNode *>(&n))    visit_number(*p);
    else if (auto *p = dynamic_cast<RealNode *>(&n))      visit_real(*p);
    else if (auto *p = dynamic_cast<CharNode *>(&n))      visit_char(*p);
    else if (auto *p = dynamic_cast<StringNode *>(&n))    visit_string(*p);
    else if (auto *p = dynamic_cast<BoolNode *>(&n))      visit_bool(*p);
    else if (auto *p = dynamic_cast<ArrayAccessNode *>(&n))  visit_array_access(*p);
    else if (auto *p = dynamic_cast<RecordAccessNode *>(&n)) visit_record_access(*p);
    else if (auto *p = dynamic_cast<FuncCallExprNode *>(&n)) visit_func_call_expr(*p);
    else
        throw SemanticError("Unknown AST node type: " + n.node_type(), n.line);
}

void SemanticVisitor::visit_number(NumberNode &n)
{
    n.type_code = TypeCode::INTEGER;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_real(RealNode &n)
{
    n.type_code = TypeCode::REAL;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_char(CharNode &n)
{
    n.type_code = TypeCode::CHAR;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_string(StringNode &n)
{
    n.type_code = TypeCode::STRING;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_bool(BoolNode &n)
{
    n.type_code = TypeCode::BOOLEAN;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_var(VarNode &n)
{
    int idx = sym.lookup(n.name);
    if (idx == -1)
        undeclared_error(n.name, n.line);

    const TabEntry &entry = sym.get_tab(idx);
    n.tab_index = idx;
    n.type_code = entry.type;
    n.lev       = entry.lev;
}

void SemanticVisitor::visit_binop(BinOpNode &n)
{
    visit(*n.left);
    visit(*n.right);

    TypeCode result = resolve_binop_type(n.op, n.left->type_code, n.right->type_code);
    if (result == TypeCode::UNKNOWN)
        type_error("Cannot apply operator '" + n.op + "' to " +
                   typecode_to_string(n.left->type_code) + " and " +
                   typecode_to_string(n.right->type_code),
                   n.line);

    n.type_code = result;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_unary(UnaryOpNode &n)
{
    visit(*n.operand);
    TypeCode t = n.operand->type_code;

    if (n.op == "not")
    {
        if (t != TypeCode::BOOLEAN)
            type_error("'not' requires Boolean operand, got " +
                       typecode_to_string(t), n.line);
        n.type_code = TypeCode::BOOLEAN;
    }
    else if (n.op == "-")
    {
        if (t != TypeCode::INTEGER && t != TypeCode::REAL)
            type_error("Unary '-' requires Integer or Real, got " +
                       typecode_to_string(t), n.line);
        n.type_code = t;
    }
    else
    {
        type_error("Unknown unary operator: " + n.op, n.line);
    }

    n.lev = sym.current_level();
}

void SemanticVisitor::visit_array_access(ArrayAccessNode &n)
{
    visit(*n.array);

    if (n.array->type_code != TypeCode::ARRAY)
        type_error("Variable is not an array", n.line);

    int atab_idx = -1;
    if (n.array->tab_index >= 0)
        atab_idx = sym.get_tab(n.array->tab_index).ref;

    for (auto &idx_expr : n.indices)
    {
        visit(*idx_expr);

        if (idx_expr->type_code == TypeCode::REAL)
            type_error("Array index cannot be Real", n.line);

        if (atab_idx >= 0)
        {
            const AtabEntry &ae = sym.get_atab(atab_idx);
            if (!is_compatible(idx_expr->type_code, ae.xtyp))
                type_error("Array index type mismatch: expected " +
                           typecode_to_string(ae.xtyp) + ", got " +
                           typecode_to_string(idx_expr->type_code), n.line);
            n.type_code = ae.etyp;
        }
        else
        {
            n.type_code = TypeCode::UNKNOWN;
        }
    }

    n.lev = sym.current_level();
}

void SemanticVisitor::visit_record_access(RecordAccessNode &n)
{
    visit(*n.record);

    if (n.record->type_code != TypeCode::RECORD)
        type_error("Variable is not a record", n.line);

    int btab_idx = -1;
    if (n.record->tab_index >= 0)
        btab_idx = sym.get_tab(n.record->tab_index).ref;

    if (btab_idx >= 0)
    {
        int field_idx = sym.get_btab(btab_idx).last;
        bool found = false;

        while (field_idx >= 0)
        {
            const TabEntry &field = sym.get_tab(field_idx);
            std::string f_lower = field.id;
            std::string s_lower = n.field_name;
            std::transform(f_lower.begin(), f_lower.end(), f_lower.begin(), ::tolower);
            std::transform(s_lower.begin(), s_lower.end(), s_lower.begin(), ::tolower);

            if (f_lower == s_lower)
            {
                n.type_code = field.type;
                n.tab_index = field_idx;
                found = true;
                break;
            }
            field_idx = field.link;
        }

        if (!found)
            type_error("Record has no field '" + n.field_name + "'", n.line);
    }
    else
    {
        n.type_code = TypeCode::UNKNOWN;
    }

    n.lev = sym.current_level();
}

void SemanticVisitor::visit_block(BlockNode &n)
{
    for (auto &stmt : n.statements)
        if (stmt) visit(*stmt);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_assign(AssignNode &n)
{
    visit(*n.target);
    visit(*n.value);

    if (!is_assignment_compatible(n.target->type_code, n.value->type_code))
        type_error("Assignment type mismatch: cannot assign " +
                   typecode_to_string(n.value->type_code) + " to " +
                   typecode_to_string(n.target->type_code),
                   n.line);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_if(IfNode &n)
{
    visit(*n.condition);
    if (n.condition->type_code != TypeCode::BOOLEAN)
        type_error("If condition must be Boolean, got " + typecode_to_string(n.condition->type_code), n.line);

    visit(*n.then_branch);
    if (n.else_branch) visit(*n.else_branch);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_while(WhileNode &n)
{
    visit(*n.condition);
    if (n.condition->type_code != TypeCode::BOOLEAN)
        type_error("While condition must be Boolean, got " +
                   typecode_to_string(n.condition->type_code), n.line);

    bool prev = inside_loop;
    inside_loop = true;
    visit(*n.body);
    inside_loop = prev;

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_for(ForNode &n)
{
    int idx = sym.lookup(n.var_name);
    if (idx == -1)
        undeclared_error(n.var_name, n.line);
    else
    {
        TypeCode var_type = sym.get_tab(idx).type;
        if (var_type != TypeCode::INTEGER && var_type != TypeCode::CHAR)
            type_error("For loop variable must be Integer or Char, got " +
                       typecode_to_string(var_type), n.line);
    }

    visit(*n.from_expr);
    visit(*n.to_expr);

    if (idx >= 0)
    {
        TypeCode vt = sym.get_tab(idx).type;
        if (!is_assignment_compatible(vt, n.from_expr->type_code))
            type_error("For loop 'from' expression type mismatch", n.line);
        if (!is_assignment_compatible(vt, n.to_expr->type_code))
            type_error("For loop 'to' expression type mismatch", n.line);
    }

    bool prev = inside_loop;
    inside_loop = true;
    visit(*n.body);
    inside_loop = prev;

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_repeat(RepeatNode &n)
{
    bool prev = inside_loop;
    inside_loop = true;
    for (auto &stmt : n.body)
        if (stmt) visit(*stmt);
    inside_loop = prev;

    visit(*n.condition);
    if (n.condition->type_code != TypeCode::BOOLEAN)
        type_error("Repeat-until condition must be Boolean, got " +
                   typecode_to_string(n.condition->type_code), n.line);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_case(CaseNode &n)
{
    visit(*n.selector);
    TypeCode sel = n.selector->type_code;

    if (sel != TypeCode::INTEGER && sel != TypeCode::CHAR && sel != TypeCode::BOOLEAN)
        type_error("Case selector must be ordinal (Integer, Char, Boolean), got " +
                   typecode_to_string(sel), n.line);

    for (auto &c : n.cases)
    {
        if (c.first)
        {
            visit(*c.first);
            if (!is_compatible(sel, c.first->type_code))
                type_error("Case value type mismatch with selector", n.line);
        }
        if (c.second) visit(*c.second);
    }

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_proc_call(ProcCallNode &n)
{
    int idx = sym.lookup(n.name);
    if (idx == -1)
        undeclared_error(n.name, n.line);
    else
    {
        const TabEntry &entry = sym.get_tab(idx);
        if (entry.obj != ObjClass::PROCEDURE)
            type_error("'" + n.name + "' is not a procedure", n.line);
        n.tab_index = idx;
    }

    for (auto &arg : n.args)
        if (arg) visit(*arg);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_func_call_expr(FuncCallExprNode &n)
{
    int idx = sym.lookup(n.name);
    if (idx == -1)
        undeclared_error(n.name, n.line);
    else
    {
        const TabEntry &entry = sym.get_tab(idx);
        if (entry.obj != ObjClass::FUNCTION)
            type_error("'" + n.name + "' is not a function", n.line);
        n.type_code = entry.type;
        n.tab_index = idx;
    }

    for (auto &arg : n.args)
        if (arg) visit(*arg);

    n.lev = sym.current_level();
}

void SemanticVisitor::visit_program(ProgramNode &n)
{
    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::PROGRAM; 
    e.type = TypeCode::VOID;
    e.ref  = -1;
    e.nrm  = 1;
    e.adr  = 0;
    sym.enter_tab(e);

    for (auto &decl : n.declarations)
        if (decl) visit(*decl);
        
    int btab_idx = sym.open_scope();
    if (n.body) {
        if (auto *blk = dynamic_cast<BlockNode*>(n.body.get()))
            blk->btab_index = btab_idx;   
        visit(*n.body);
    }
    sym.close_scope();

    n.type_code = TypeCode::VOID;
    n.lev = 0;
}

void SemanticVisitor::visit_var_decl(VarDeclNode &n)
{
    if (n.type_node) visit(*n.type_node);
    TypeCode t = n.type_node ? n.type_node->type_code : TypeCode::UNKNOWN;

    for (const auto &name : n.names)
    {
        if (sym.lookup_in_level(name, sym.current_level()) != -1)
            redeclared_error(name, n.line);

        TabEntry e;
        e.id   = name;
        e.obj  = ObjClass::VARIABLE;
        e.type = t;
        e.ref  = n.type_node ? n.type_node->tab_index : -1;
        e.nrm  = 1;
        e.adr  = sym.current_btab_index() >= 0
                     ? sym.get_btab(sym.current_btab_index()).vsze
                     : 0;
        sym.enter_tab(e);

        if (sym.current_btab_index() >= 0)
            sym.get_btab(sym.current_btab_index()).vsze += 1;
    }

    n.lev = sym.current_level();
    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_const_decl(ConstDeclNode &n)
{
    if (n.value) visit(*n.value);

    if (sym.lookup_in_level(n.name, sym.current_level()) != -1)
        redeclared_error(n.name, n.line);

    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::CONSTANT;
    e.type = n.value ? n.value->type_code : TypeCode::UNKNOWN;
    e.ref  = -1;
    e.nrm  = 1;
    e.adr  = 0;
    sym.enter_tab(e);

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_type_decl(TypeDeclNode &n)
{
    if (n.type_def) visit(*n.type_def);

    if (sym.lookup_in_level(n.name, sym.current_level()) != -1)
        redeclared_error(n.name, n.line);

    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::TYPE;
    e.type = n.type_def ? n.type_def->type_code : TypeCode::UNKNOWN;
    e.ref  = n.type_def ? n.type_def->tab_index : -1;
    e.nrm  = 1;
    e.adr  = 0;
    sym.enter_tab(e);

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_proc_decl(ProcDeclNode &n)
{
    if (sym.lookup_in_level(n.name, sym.current_level()) != -1)
        redeclared_error(n.name, n.line);

    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::PROCEDURE;
    e.type = TypeCode::VOID;
    e.ref  = -1;
    e.nrm  = 1;
    e.adr  = 0;
    int proc_idx = sym.enter_tab(e);

    int btab_idx = sym.open_scope();
    sym.get_tab(proc_idx).ref = btab_idx;

    for (auto &param : n.params)
        if (param) visit(*param);

    sym.get_btab(btab_idx).lpar = sym.get_btab(btab_idx).last;

    if (n.body) visit(*n.body);

    sym.close_scope();
    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_func_decl(FuncDeclNode &n)
{
    if (n.return_type) visit(*n.return_type);
    TypeCode ret = n.return_type ? n.return_type->type_code : TypeCode::UNKNOWN;

    if (sym.lookup_in_level(n.name, sym.current_level()) != -1)
        redeclared_error(n.name, n.line);

    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::FUNCTION;
    e.type = ret;
    e.ref  = -1;
    e.nrm  = 1;
    e.adr  = 0;
    int func_idx = sym.enter_tab(e);

    int btab_idx = sym.open_scope();
    sym.get_tab(func_idx).ref = btab_idx;

    for (auto &param : n.params)
        if (param) visit(*param);

    sym.get_btab(btab_idx).lpar = sym.get_btab(btab_idx).last;

    if (n.body) visit(*n.body);

    sym.close_scope();
    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_param_group(ParamGroupNode &n)
{
    if (n.type_node) visit(*n.type_node);
    TypeCode t = n.type_node ? n.type_node->type_code : TypeCode::UNKNOWN;

    for (const auto &name : n.names)
    {
        TabEntry e;
        e.id   = name;
        e.obj  = ObjClass::VARIABLE;
        e.type = t;
        e.ref  = -1;
        e.nrm  = n.is_var_param ? 0 : 1;
        e.adr  = sym.current_btab_index() >= 0
                     ? sym.get_btab(sym.current_btab_index()).psze
                     : 0;
        sym.enter_tab(e);

        if (sym.current_btab_index() >= 0)
            sym.get_btab(sym.current_btab_index()).psze += 1;
    }

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_array_type(ArrayTypeNode &n)
{
    if (n.index_range) visit(*n.index_range);
    if (n.elem_type)   visit(*n.elem_type);

    if (n.index_range && n.index_range->type_code == TypeCode::REAL)
        type_error("Array index type cannot be Real", n.line);

    AtabEntry ae;
    ae.xtyp = n.index_range ? n.index_range->type_code : TypeCode::INTEGER;
    ae.etyp = n.elem_type   ? n.elem_type->type_code   : TypeCode::UNKNOWN;
    ae.eref = n.elem_type   ? n.elem_type->tab_index    : -1;
    ae.elsz = 1;
    ae.low  = 0;
    ae.high = 0;

    if (n.index_range)
    {
        if (auto *sub = dynamic_cast<SubrangeNode *>(n.index_range.get()))
        {
            if (auto *lo = dynamic_cast<NumberNode *>(sub->low.get()))
                ae.low = lo->value;
            if (auto *hi = dynamic_cast<NumberNode *>(sub->high.get()))
                ae.high = hi->value;
        }
    }
    ae.size = (ae.high - ae.low + 1) * ae.elsz;

    int atab_idx = sym.enter_atab(ae);
    n.type_code = TypeCode::ARRAY;
    n.tab_index = atab_idx;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_record_type(RecordTypeNode &n)
{
    int btab_idx = sym.open_scope();

    for (auto &field : n.fields)
        if (field) visit(*field);

    sym.close_scope();

    n.type_code = TypeCode::RECORD;
    n.tab_index = btab_idx;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_subrange(SubrangeNode &n)
{
    if (n.low)  visit(*n.low);
    if (n.high) visit(*n.high);

    if ((n.low  && n.low->type_code  == TypeCode::REAL) ||
        (n.high && n.high->type_code == TypeCode::REAL))
        type_error("Subrange type cannot be Real", n.line);

    if (n.low && n.high)
    {
        if (!is_compatible(n.low->type_code, n.high->type_code))
            type_error("Subrange bounds must have compatible types", n.line);
        n.type_code = n.low->type_code;
    }
    else
    {
        n.type_code = TypeCode::INTEGER;
    }

    if (n.type_code == TypeCode::INTEGER)
    {
        auto *lo = dynamic_cast<NumberNode *>(n.low.get());
        auto *hi = dynamic_cast<NumberNode *>(n.high.get());
        if (lo && hi && lo->value > hi->value)
            type_error("Subrange lower bound must not exceed upper bound", n.line);
    }

    n.lev = sym.current_level();
}

void SemanticVisitor::visit_enum(EnumNode &n)
{
    for (const auto &val : n.values)
    {
        if (sym.lookup_in_level(val, sym.current_level()) != -1)
            redeclared_error(val, n.line);

        TabEntry e;
        e.id   = val;
        e.obj  = ObjClass::CONSTANT;
        e.type = TypeCode::ENUM;
        e.ref  = -1;
        e.nrm  = 1;
        e.adr  = 0;
        sym.enter_tab(e);
    }

    n.type_code = TypeCode::ENUM;
    n.lev = sym.current_level();
}

bool SemanticVisitor::is_compatible(TypeCode t1, TypeCode t2) const
{
    if (t1 == t2) return true;
    if ((t1 == TypeCode::SUBRANGE && t2 == TypeCode::INTEGER) ||
        (t1 == TypeCode::INTEGER  && t2 == TypeCode::SUBRANGE) ||
        (t1 == TypeCode::SUBRANGE && t2 == TypeCode::SUBRANGE))
        return true;
    return false;
}

bool SemanticVisitor::is_assignment_compatible(TypeCode t1, TypeCode t2) const
{
    if (t1 == TypeCode::REAL && t2 == TypeCode::INTEGER) return true;
    return is_compatible(t1, t2);
}

TypeCode SemanticVisitor::resolve_binop_type(const std::string &op, TypeCode lt, TypeCode rt) const
{
    if (op == "+" || op == "-" || op == "*")
    {
        if (lt == TypeCode::INTEGER && rt == TypeCode::INTEGER) return TypeCode::INTEGER;
        if ((lt == TypeCode::REAL || lt == TypeCode::INTEGER) &&
            (rt == TypeCode::REAL || rt == TypeCode::INTEGER)) return TypeCode::REAL;
        return TypeCode::UNKNOWN;
    }
    if (op == "/")
    {
        if ((lt == TypeCode::INTEGER || lt == TypeCode::REAL) &&
            (rt == TypeCode::INTEGER || rt == TypeCode::REAL)) return TypeCode::REAL;
        return TypeCode::UNKNOWN;
    }
    if (op == "div" || op == "mod")
    {
        if (lt == TypeCode::INTEGER && rt == TypeCode::INTEGER) return TypeCode::INTEGER;
        return TypeCode::UNKNOWN;
    }
    if (op == "and" || op == "or")
    {
        if (lt == TypeCode::BOOLEAN && rt == TypeCode::BOOLEAN) return TypeCode::BOOLEAN;
        return TypeCode::UNKNOWN;
    }
    if (op == "==" || op == "<>" || op == "<" ||
        op == ">"  || op == "<=" || op == ">=")
    {

        auto is_ordinal = [](TypeCode t) {
            return t == TypeCode::INTEGER || t == TypeCode::REAL   ||
                   t == TypeCode::CHAR    || t == TypeCode::STRING ||
                   t == TypeCode::SUBRANGE;
        };
        if (is_compatible(lt, rt) && is_ordinal(lt)) return TypeCode::BOOLEAN;
        if ((lt == TypeCode::INTEGER || lt == TypeCode::REAL) &&
            (rt == TypeCode::INTEGER || rt == TypeCode::REAL))  return TypeCode::BOOLEAN;
        if (lt == TypeCode::CHAR   && rt == TypeCode::CHAR)     return TypeCode::BOOLEAN;
        if (lt == TypeCode::STRING && rt == TypeCode::STRING)   return TypeCode::BOOLEAN;
        return TypeCode::UNKNOWN;
    }
    return TypeCode::UNKNOWN;
}

void SemanticVisitor::type_error(const std::string &msg, int line) const
{
    throw SemanticError(msg, line);
}

void SemanticVisitor::undeclared_error(const std::string &name, int line) const
{
    throw SemanticError("Undeclared identifier: '" + name + "'", line);
}

void SemanticVisitor::redeclared_error(const std::string &name, int line) const
{
    throw SemanticError("Identifier already declared in this scope: '" + name + "'", line);
}