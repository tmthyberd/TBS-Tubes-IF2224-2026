#include "SemanticVisitor.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

// ============================================================
// Constructor
// ============================================================

SemanticVisitor::SemanticVisitor(SymbolTable &sym) : sym(sym) {}

// ============================================================
// visit() — Dispatcher utama
// Setiap node AST dipetakan ke fungsi visit yang sesuai
// ============================================================

void SemanticVisitor::visit(ASTNode &n)
{
    if (auto *p = dynamic_cast<ProgramNode *>(&n))
        visit_program(*p);
    else if (auto *p = dynamic_cast<VarDeclNode *>(&n))
        visit_var_decl(*p);
    else if (auto *p = dynamic_cast<ConstDeclNode *>(&n))
        visit_const_decl(*p);
    else if (auto *p = dynamic_cast<TypeDeclNode *>(&n))
        visit_type_decl(*p);
    else if (auto *p = dynamic_cast<ProcDeclNode *>(&n))
        visit_proc_decl(*p);
    else if (auto *p = dynamic_cast<FuncDeclNode *>(&n))
        visit_func_decl(*p);
    else if (auto *p = dynamic_cast<ParamGroupNode *>(&n))
        visit_param_group(*p);
    else if (auto *p = dynamic_cast<ArrayTypeNode *>(&n))
        visit_array_type(*p);
    else if (auto *p = dynamic_cast<RecordTypeNode *>(&n))
        visit_record_type(*p);
    else if (auto *p = dynamic_cast<SubrangeNode *>(&n))
        visit_subrange(*p);
    else if (auto *p = dynamic_cast<EnumNode *>(&n))
        visit_enum(*p);
    else if (auto *p = dynamic_cast<BlockNode *>(&n))
        visit_block(*p);
    else if (auto *p = dynamic_cast<AssignNode *>(&n))
        visit_assign(*p);
    else if (auto *p = dynamic_cast<IfNode *>(&n))
        visit_if(*p);
    else if (auto *p = dynamic_cast<WhileNode *>(&n))
        visit_while(*p);
    else if (auto *p = dynamic_cast<ForNode *>(&n))
        visit_for(*p);
    else if (auto *p = dynamic_cast<RepeatNode *>(&n))
        visit_repeat(*p);
    else if (auto *p = dynamic_cast<CaseNode *>(&n))
        visit_case(*p);
    else if (auto *p = dynamic_cast<ProcCallNode *>(&n))
        visit_proc_call(*p);
    else if (auto *p = dynamic_cast<BinOpNode *>(&n))
        visit_binop(*p);
    else if (auto *p = dynamic_cast<UnaryOpNode *>(&n))
        visit_unary(*p);
    else if (auto *p = dynamic_cast<VarNode *>(&n))
        visit_var(*p);
    else if (auto *p = dynamic_cast<NumberNode *>(&n))
        visit_number(*p);
    else if (auto *p = dynamic_cast<RealNode *>(&n))
        visit_real(*p);
    else if (auto *p = dynamic_cast<CharNode *>(&n))
        visit_char(*p);
    else if (auto *p = dynamic_cast<StringNode *>(&n))
        visit_string(*p);
    else if (auto *p = dynamic_cast<BoolNode *>(&n))
        visit_bool(*p);
    else if (auto *p = dynamic_cast<ArrayAccessNode *>(&n))
        visit_array_access(*p);
    else if (auto *p = dynamic_cast<RecordAccessNode *>(&n))
        visit_record_access(*p);
    else if (auto *p = dynamic_cast<FuncCallExprNode *>(&n))
        visit_func_call_expr(*p);
    else
        throw SemanticError("Unknown AST node type: " + n.node_type(), n.line);
}

// ============================================================
// LITERAL NODES — Hanya set tipe, tidak ada pengecekan
// ============================================================

void SemanticVisitor::visit_number(NumberNode &n)
{
    // intcon → INTEGER
    n.type_code = TypeCode::INTEGER;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_real(RealNode &n)
{
    // realcon → REAL
    n.type_code = TypeCode::REAL;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_char(CharNode &n)
{
    // charcon → CHAR
    n.type_code = TypeCode::CHAR;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_string(StringNode &n)
{
    // string literal → STRING
    n.type_code = TypeCode::STRING;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_bool(BoolNode &n)
{
    // true / false → BOOLEAN
    n.type_code = TypeCode::BOOLEAN;
    n.lev = sym.current_level();
}

// ============================================================
// VARIABLE — Lookup di symbol table
// ============================================================

void SemanticVisitor::visit_var(VarNode &n)
{
    // Lookup identifier di symbol table (case-insensitive, QnA M3 no.1)
    std::string lower_name = n.name;
    std::transform(lower_name.begin(), lower_name.end(),
                   lower_name.begin(), ::tolower);

    int idx = sym.lookup(lower_name);
    if (idx == -1)
    {
        // Coba lookup dengan nama asli (tidak lowercase)
        idx = sym.lookup(n.name);
        if (idx == -1)
            undeclared_error(n.name, n.line);
    }

    // Anotasi node dengan info dari symbol table
    const TabEntry &entry = sym.get_tab(idx);
    n.tab_index = idx;
    n.type_code = entry.type;
    n.lev = entry.lev;
}

// ============================================================
// EKSPRESI — BinOp dan Unary
// ============================================================

void SemanticVisitor::visit_binop(BinOpNode &n)
{
    // Visit kiri dan kanan dulu (synthesized attributes)
    visit(*n.left);
    visit(*n.right);

    TypeCode left_t  = n.left->type_code;
    TypeCode right_t = n.right->type_code;

    // Hitung tipe hasil berdasarkan operator dan tipe operand
    TypeCode result = resolve_binop_type(n.op, left_t, right_t);
    if (result == TypeCode::UNKNOWN)
    {
        type_error("Cannot apply operator '" + n.op + "' to " +
                   typecode_to_string(left_t) + " and " +
                   typecode_to_string(right_t),
                   n.line);
    }

    n.type_code = result;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_unary(UnaryOpNode &n)
{
    // Visit operand dulu
    visit(*n.operand);

    TypeCode t = n.operand->type_code;

    if (n.op == "not")
    {
        // not hanya valid untuk BOOLEAN
        if (t != TypeCode::BOOLEAN)
            type_error("'not' requires Boolean operand, got " +
                       typecode_to_string(t), n.line);
        n.type_code = TypeCode::BOOLEAN;
    }
    else if (n.op == "-")
    {
        // Unary minus hanya valid untuk INTEGER atau REAL
        if (t != TypeCode::INTEGER && t != TypeCode::REAL)
            type_error("Unary '-' requires Integer or Real operand, got " +
                       typecode_to_string(t), n.line);
        n.type_code = t; // -integer = integer, -real = real
    }
    else
    {
        type_error("Unknown unary operator: " + n.op, n.line);
    }

    n.lev = sym.current_level();
}

// ============================================================
// VARIABLE ACCESS — Array dan Record
// ============================================================

void SemanticVisitor::visit_array_access(ArrayAccessNode &n)
{
    // Visit array expression (harus bertipe ARRAY)
    visit(*n.array);

    if (n.array->type_code != TypeCode::ARRAY)
        type_error("Variable is not an array", n.line);

    // Dapatkan info array dari atab
    int atab_idx = -1;
    if (n.array->tab_index >= 0)
    {
        const TabEntry &arr_entry = sym.get_tab(n.array->tab_index);
        atab_idx = arr_entry.ref;
    }

    // Visit setiap index dan validasi tipenya
    for (auto &idx_expr : n.indices)
    {
        visit(*idx_expr);
        TypeCode idx_type = idx_expr->type_code;

        // Index tidak boleh REAL (QnA spesifikasi)
        if (idx_type == TypeCode::REAL)
            type_error("Array index cannot be of type Real", n.line);

        // Validasi kompatibilitas tipe index dengan atab.xtyp
        if (atab_idx >= 0)
        {
            const AtabEntry &atab = sym.get_atab(atab_idx);
            if (!is_compatible(idx_type, atab.xtyp))
                type_error("Array index type mismatch: expected " +
                           typecode_to_string(atab.xtyp) + ", got " +
                           typecode_to_string(idx_type), n.line);
            // Set tipe hasil = tipe elemen array
            n.type_code = atab.etyp;
        }
        else
        {
            // Tidak ada info atab, set ke UNKNOWN (akan di-handle nanti)
            n.type_code = TypeCode::UNKNOWN;
        }
    }

    n.lev = sym.current_level();
}

void SemanticVisitor::visit_record_access(RecordAccessNode &n)
{
    // Visit record expression dulu
    visit(*n.record);

    if (n.record->type_code != TypeCode::RECORD)
        type_error("Variable is not a record", n.line);

    // Cari field di btab record
    int btab_idx = -1;
    if (n.record->tab_index >= 0)
    {
        const TabEntry &rec_entry = sym.get_tab(n.record->tab_index);
        btab_idx = rec_entry.ref;
    }

    if (btab_idx >= 0)
    {
        // Iterasi field di blok record lewat linked list tab
        const BtabEntry &btab = sym.get_btab(btab_idx);
        int field_idx = btab.last;
        bool found = false;

        while (field_idx > 0)
        {
            const TabEntry &field = sym.get_tab(field_idx);
            std::string lower_field = field.id;
            std::string lower_search = n.field_name;
            std::transform(lower_field.begin(), lower_field.end(),
                           lower_field.begin(), ::tolower);
            std::transform(lower_search.begin(), lower_search.end(),
                           lower_search.begin(), ::tolower);

            if (lower_field == lower_search)
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

// ============================================================
// STATEMENTS
// ============================================================

void SemanticVisitor::visit_block(BlockNode &n)
{
    // Visit setiap statement dalam blok
    for (auto &stmt : n.statements)
        if (stmt) visit(*stmt);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_assign(AssignNode &n)
{
    // Visit target (sisi kiri) dan value (sisi kanan)
    visit(*n.target);
    visit(*n.value);

    TypeCode t_target = n.target->type_code;
    TypeCode t_value  = n.value->type_code;

    // Cek assignment-compatible
    if (!is_assignment_compatible(t_target, t_value))
    {
        type_error("Assignment type mismatch: cannot assign " +
                   typecode_to_string(t_value) + " to " +
                   typecode_to_string(t_target),
                   n.line);
    }

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_if(IfNode &n)
{
    // Visit kondisi — harus BOOLEAN
    visit(*n.condition);
    if (n.condition->type_code != TypeCode::BOOLEAN)
        type_error("If condition must be Boolean, got " +
                   typecode_to_string(n.condition->type_code), n.line);

    // Visit then-branch
    visit(*n.then_branch);

    // Visit else-branch jika ada
    if (n.else_branch)
        visit(*n.else_branch);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_while(WhileNode &n)
{
    // Visit kondisi — harus BOOLEAN
    visit(*n.condition);
    if (n.condition->type_code != TypeCode::BOOLEAN)
        type_error("While condition must be Boolean, got " +
                   typecode_to_string(n.condition->type_code), n.line);

    // Tandai sedang di dalam loop
    bool prev = inside_loop;
    inside_loop = true;
    visit(*n.body);
    inside_loop = prev;

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_for(ForNode &n)
{
    // Lookup variabel kontrol loop
    int idx = sym.lookup(n.var_name);
    if (idx == -1)
        undeclared_error(n.var_name, n.line);
    else
    {
        // Variabel kontrol harus bertipe ordinal (INTEGER atau CHAR)
        TypeCode var_type = sym.get_tab(idx).type;
        if (var_type != TypeCode::INTEGER && var_type != TypeCode::CHAR)
            type_error("For loop variable must be Integer or Char, got " +
                       typecode_to_string(var_type), n.line);
    }

    // Visit ekspresi from dan to
    visit(*n.from_expr);
    visit(*n.to_expr);

    // from dan to harus assignment-compatible dengan variabel kontrol
    if (idx >= 0)
    {
        TypeCode var_type = sym.get_tab(idx).type;
        if (!is_assignment_compatible(var_type, n.from_expr->type_code))
            type_error("For loop 'from' expression type mismatch", n.line);
        if (!is_assignment_compatible(var_type, n.to_expr->type_code))
            type_error("For loop 'to' expression type mismatch", n.line);
    }

    // Visit body (compound-statement)
    bool prev = inside_loop;
    inside_loop = true;
    visit(*n.body);
    inside_loop = prev;

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_repeat(RepeatNode &n)
{
    // Visit body terlebih dahulu (repeat diperiksa setelah body)
    bool prev = inside_loop;
    inside_loop = true;
    for (auto &stmt : n.body)
        if (stmt) visit(*stmt);
    inside_loop = prev;

    // Visit kondisi — harus BOOLEAN
    visit(*n.condition);
    if (n.condition->type_code != TypeCode::BOOLEAN)
        type_error("Repeat-until condition must be Boolean, got " +
                   typecode_to_string(n.condition->type_code), n.line);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_case(CaseNode &n)
{
    // Visit selector
    visit(*n.selector);
    TypeCode sel_type = n.selector->type_code;

    // Selector harus ordinal: INTEGER, CHAR, atau BOOLEAN
    if (sel_type != TypeCode::INTEGER &&
        sel_type != TypeCode::CHAR &&
        sel_type != TypeCode::BOOLEAN)
    {
        type_error("Case selector must be an ordinal type (Integer, Char, Boolean), got " +
                   typecode_to_string(sel_type), n.line);
    }

    // Visit setiap pasangan (nilai, statement)
    for (auto &c : n.cases)
    {
        if (c.first)
        {
            visit(*c.first); // kunjungi nilai case
            // Tipe nilai harus kompatibel dengan selector
            if (!is_compatible(sel_type, c.first->type_code))
                type_error("Case value type mismatch with selector", n.line);
        }
        if (c.second)
            visit(*c.second); // kunjungi statement
    }

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

// ============================================================
// PROCEDURE / FUNCTION CALL
// ============================================================

void SemanticVisitor::visit_proc_call(ProcCallNode &n)
{
    // Lookup nama prosedur di symbol table (case-insensitive)
    std::string lower_name = n.name;
    std::transform(lower_name.begin(), lower_name.end(),
                   lower_name.begin(), ::tolower);

    int idx = sym.lookup(lower_name);
    if (idx == -1) idx = sym.lookup(n.name);
    if (idx == -1)
        undeclared_error(n.name, n.line);
    else
    {
        const TabEntry &entry = sym.get_tab(idx);
        // Validasi bahwa ini memang PROCEDURE (bukan FUNCTION atau VARIABLE)
        if (entry.obj != ObjClass::PROCEDURE)
            type_error("'" + n.name + "' is not a procedure", n.line);

        n.tab_index = idx;
    }

    // Visit setiap argumen
    for (auto &arg : n.args)
        if (arg) visit(*arg);

    n.type_code = TypeCode::VOID;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_func_call_expr(FuncCallExprNode &n)
{
    // Lookup nama fungsi di symbol table (case-insensitive)
    std::string lower_name = n.name;
    std::transform(lower_name.begin(), lower_name.end(),
                   lower_name.begin(), ::tolower);

    int idx = sym.lookup(lower_name);
    if (idx == -1) idx = sym.lookup(n.name);
    if (idx == -1)
        undeclared_error(n.name, n.line);
    else
    {
        const TabEntry &entry = sym.get_tab(idx);
        // Validasi bahwa ini memang FUNCTION
        if (entry.obj != ObjClass::FUNCTION)
            type_error("'" + n.name + "' is not a function", n.line);

        // Set tipe return dari fungsi
        n.type_code = entry.type;
        n.tab_index = idx;
    }

    // Visit setiap argumen
    for (auto &arg : n.args)
        if (arg) visit(*arg);

    n.lev = sym.current_level();
}

// ============================================================
// VISITOR UNTUK DEKLARASI
// (Bagian ini akan diimplementasikan oleh Naufal,
//  stub ini agar kode bisa dikompilasi)
// ============================================================

void SemanticVisitor::visit_program(ProgramNode &n)
{
    // Daftarkan nama program ke tab
    TabEntry prog_entry;
    prog_entry.id   = n.name;
    prog_entry.obj  = ObjClass::PROCEDURE; // program diperlakukan seperti prosedur
    prog_entry.type = TypeCode::VOID;
    prog_entry.lev  = 0;
    prog_entry.link = 0;
    prog_entry.ref  = 0;
    prog_entry.nrm  = 1;
    prog_entry.adr  = 0;
    sym.enter_tab(prog_entry);

    // Visit semua deklarasi
    for (auto &decl : n.declarations)
        if (decl) visit(*decl);

    // Visit body program
    if (n.body) visit(*n.body);

    n.type_code = TypeCode::VOID;
    n.lev = 0;
}

void SemanticVisitor::visit_var_decl(VarDeclNode &n)
{
    // Visit tipe terlebih dahulu
    if (n.type_node) visit(*n.type_node);

    TypeCode t = n.type_node ? n.type_node->type_code : TypeCode::UNKNOWN;

    // Daftarkan setiap nama variabel ke tab
    for (const auto &name : n.names)
    {
        // Cek duplikat di scope saat ini
        int dup = sym.lookup_in_level(name, sym.current_level());
        if (dup != -1)
            redeclared_error(name, n.line);

        TabEntry e;
        e.id   = name;
        e.obj  = ObjClass::VARIABLE;
        e.type = t;
        e.lev  = sym.current_level();
        e.link = sym.current_btab_index() >= 0
                     ? sym.get_btab(sym.current_btab_index()).last
                     : 0;
        e.ref  = n.type_node ? n.type_node->tab_index : 0;
        e.nrm  = 1;
        e.adr  = sym.get_btab(sym.current_btab_index()).vsze;

        int new_idx = sym.enter_tab(e);

        // Update btab
        if (sym.current_btab_index() >= 0)
        {
            BtabEntry &blk = sym.get_btab(sym.current_btab_index());
            blk.last  = new_idx;
            blk.vsze += 1; // ukuran per variabel = 1 unit
        }
    }

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_const_decl(ConstDeclNode &n)
{
    // Visit value dulu
    if (n.value) visit(*n.value);

    // Cek duplikat
    int dup = sym.lookup_in_level(n.name, sym.current_level());
    if (dup != -1)
        redeclared_error(n.name, n.line);

    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::CONSTANT;
    e.type = n.value ? n.value->type_code : TypeCode::UNKNOWN;
    e.lev  = sym.current_level();
    e.link = sym.current_btab_index() >= 0
                 ? sym.get_btab(sym.current_btab_index()).last
                 : 0;
    e.ref  = 0;
    e.nrm  = 1;
    e.adr  = 0;

    int new_idx = sym.enter_tab(e);

    if (sym.current_btab_index() >= 0)
        sym.get_btab(sym.current_btab_index()).last = new_idx;

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_type_decl(TypeDeclNode &n)
{
    // Visit definisi tipe
    if (n.type_def) visit(*n.type_def);

    // Cek duplikat
    int dup = sym.lookup_in_level(n.name, sym.current_level());
    if (dup != -1)
        redeclared_error(n.name, n.line);

    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::TYPE;
    e.type = n.type_def ? n.type_def->type_code : TypeCode::UNKNOWN;
    e.lev  = sym.current_level();
    e.link = sym.current_btab_index() >= 0
                 ? sym.get_btab(sym.current_btab_index()).last
                 : 0;
    e.ref  = n.type_def ? n.type_def->tab_index : 0;
    e.nrm  = 1;
    e.adr  = 0;

    int new_idx = sym.enter_tab(e);

    if (sym.current_btab_index() >= 0)
        sym.get_btab(sym.current_btab_index()).last = new_idx;

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_proc_decl(ProcDeclNode &n)
{
    // Cek duplikat
    int dup = sym.lookup_in_level(n.name, sym.current_level());
    if (dup != -1)
        redeclared_error(n.name, n.line);

    // Daftarkan prosedur ke tab
    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::PROCEDURE;
    e.type = TypeCode::VOID;
    e.lev  = sym.current_level();
    e.link = sym.current_btab_index() >= 0
                 ? sym.get_btab(sym.current_btab_index()).last
                 : 0;
    e.nrm  = 1;
    e.adr  = 0;

    int proc_idx = sym.enter_tab(e);
    if (sym.current_btab_index() >= 0)
        sym.get_btab(sym.current_btab_index()).last = proc_idx;

    // Buka scope baru untuk body prosedur
    int btab_idx = sym.open_scope();
    sym.get_tab(proc_idx).ref = btab_idx;

    // Visit parameter
    for (auto &param : n.params)
        if (param) visit(*param);

    // Update lpar di btab
    sym.get_btab(btab_idx).lpar = sym.get_btab(btab_idx).last;

    // Visit body
    if (n.body) visit(*n.body);

    sym.close_scope();
    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_func_decl(FuncDeclNode &n)
{
    // Cek duplikat
    int dup = sym.lookup_in_level(n.name, sym.current_level());
    if (dup != -1)
        redeclared_error(n.name, n.line);

    // Visit tipe return dulu untuk dapatkan type_code-nya
    if (n.return_type) visit(*n.return_type);
    TypeCode ret_type = n.return_type
                            ? n.return_type->type_code
                            : TypeCode::UNKNOWN;

    // Daftarkan fungsi ke tab
    TabEntry e;
    e.id   = n.name;
    e.obj  = ObjClass::FUNCTION;
    e.type = ret_type;
    e.lev  = sym.current_level();
    e.link = sym.current_btab_index() >= 0
                 ? sym.get_btab(sym.current_btab_index()).last
                 : 0;
    e.nrm  = 1;
    e.adr  = 0;

    int func_idx = sym.enter_tab(e);
    if (sym.current_btab_index() >= 0)
        sym.get_btab(sym.current_btab_index()).last = func_idx;

    // Buka scope baru
    int btab_idx = sym.open_scope();
    sym.get_tab(func_idx).ref = btab_idx;

    // Visit parameter
    for (auto &param : n.params)
        if (param) visit(*param);

    sym.get_btab(btab_idx).lpar = sym.get_btab(btab_idx).last;

    // Visit body
    if (n.body) visit(*n.body);

    sym.close_scope();
    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_param_group(ParamGroupNode &n)
{
    // Visit tipe parameter
    if (n.type_node) visit(*n.type_node);
    TypeCode t = n.type_node ? n.type_node->type_code : TypeCode::UNKNOWN;

    // Daftarkan setiap nama parameter
    for (const auto &name : n.names)
    {
        TabEntry e;
        e.id   = name;
        e.obj  = ObjClass::VARIABLE;
        e.type = t;
        e.lev  = sym.current_level();
        e.link = sym.current_btab_index() >= 0
                     ? sym.get_btab(sym.current_btab_index()).last
                     : 0;
        e.ref  = 0;
        e.nrm  = n.is_var_param ? 0 : 1; // 0 = by-reference, 1 = normal
        e.adr  = sym.get_btab(sym.current_btab_index()).psze;

        int new_idx = sym.enter_tab(e);

        if (sym.current_btab_index() >= 0)
        {
            BtabEntry &blk = sym.get_btab(sym.current_btab_index());
            blk.last  = new_idx;
            blk.psze += 1;
        }
    }

    n.type_code = TypeCode::VOID;
}

void SemanticVisitor::visit_array_type(ArrayTypeNode &n)
{
    // Visit index range dan element type
    if (n.index_range) visit(*n.index_range);
    if (n.elem_type)   visit(*n.elem_type);

    // Index type tidak boleh REAL
    if (n.index_range &&
        n.index_range->type_code == TypeCode::REAL)
    {
        type_error("Array index type cannot be Real", n.line);
    }

    // Buat entri di atab
    AtabEntry ae;
    ae.xtyp = n.index_range ? n.index_range->type_code : TypeCode::INTEGER;
    ae.etyp = n.elem_type   ? n.elem_type->type_code   : TypeCode::UNKNOWN;
    ae.eref = n.elem_type   ? n.elem_type->tab_index    : -1;
    ae.elsz = 1;

    // Dapatkan batas bawah dan atas dari SubrangeNode jika ada
    ae.low  = 0;
    ae.high = 0;
    if (n.index_range)
    {
        if (auto *sub = dynamic_cast<SubrangeNode *>(n.index_range.get()))
        {
            if (sub->low)
            {
                if (auto *num = dynamic_cast<NumberNode *>(sub->low.get()))
                    ae.low = num->value;
            }
            if (sub->high)
            {
                if (auto *num = dynamic_cast<NumberNode *>(sub->high.get()))
                    ae.high = num->value;
            }
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
    // Buka scope baru untuk field record
    int btab_idx = sym.open_scope();

    // Visit setiap field
    for (auto &field : n.fields)
        if (field) visit(*field);

    sym.close_scope();

    n.type_code = TypeCode::RECORD;
    n.tab_index = btab_idx;
    n.lev = sym.current_level();
}

void SemanticVisitor::visit_subrange(SubrangeNode &n)
{
    // Visit batas bawah dan atas
    if (n.low)  visit(*n.low);
    if (n.high) visit(*n.high);

    // Subrange tidak boleh bertipe REAL
    if ((n.low  && n.low->type_code  == TypeCode::REAL) ||
        (n.high && n.high->type_code == TypeCode::REAL))
    {
        type_error("Subrange type cannot be Real", n.line);
    }

    // Tipe subrange ditentukan oleh tipe konstanta di dalamnya
    if (n.low && n.high)
    {
        if (!is_compatible(n.low->type_code, n.high->type_code))
            type_error("Subrange bounds must have compatible types", n.line);
        n.type_code = n.low->type_code;
    }
    else
    {
        n.type_code = TypeCode::INTEGER; // default
    }

    // Validasi low <= high (hanya untuk integer)
    if (n.low && n.high &&
        n.type_code == TypeCode::INTEGER)
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
    // Daftarkan setiap nilai enum ke tab sebagai konstanta
    for (const auto &val : n.values)
    {
        int dup = sym.lookup_in_level(val, sym.current_level());
        if (dup != -1)
            redeclared_error(val, n.line);

        TabEntry e;
        e.id   = val;
        e.obj  = ObjClass::CONSTANT;
        e.type = TypeCode::ENUM;
        e.lev  = sym.current_level();
        e.link = sym.current_btab_index() >= 0
                     ? sym.get_btab(sym.current_btab_index()).last
                     : 0;
        e.ref  = 0;
        e.nrm  = 1;
        e.adr  = 0;

        int new_idx = sym.enter_tab(e);
        if (sym.current_btab_index() >= 0)
            sym.get_btab(sym.current_btab_index()).last = new_idx;
    }

    n.type_code = TypeCode::ENUM;
    n.lev = sym.current_level();
}

// ============================================================
// HELPER FUNCTIONS
// ============================================================

bool SemanticVisitor::is_compatible(TypeCode t1, TypeCode t2) const
{
    // Dua tipe compatible jika:
    // 1. Sama persis
    if (t1 == t2) return true;

    // 2. Salah satu SUBRANGE, yang lain INTEGER (atau keduanya SUBRANGE)
    if ((t1 == TypeCode::SUBRANGE && t2 == TypeCode::INTEGER) ||
        (t1 == TypeCode::INTEGER  && t2 == TypeCode::SUBRANGE) ||
        (t1 == TypeCode::SUBRANGE && t2 == TypeCode::SUBRANGE))
        return true;

    // 3. STRING dengan STRING (dianggap compatible — panjang tidak dicek di sini)
    if (t1 == TypeCode::STRING && t2 == TypeCode::STRING) return true;

    return false;
}

bool SemanticVisitor::is_assignment_compatible(TypeCode t1, TypeCode t2) const
{
    // T2 assignment-compatible ke T1 jika:
    // 1. T1 = REAL dan T2 = INTEGER (integer bisa masuk real)
    if (t1 == TypeCode::REAL && t2 == TypeCode::INTEGER) return true;

    // 2. Keduanya compatible
    if (is_compatible(t1, t2)) return true;

    // 3. Keduanya STRING
    if (t1 == TypeCode::STRING && t2 == TypeCode::STRING) return true;

    return false;
}

TypeCode SemanticVisitor::resolve_binop_type(const std::string &op,
                                              TypeCode left_type,
                                              TypeCode right_type) const
{
    // Operator aritmatika: +, -, *
    if (op == "+" || op == "-" || op == "*")
    {
        if (left_type == TypeCode::INTEGER && right_type == TypeCode::INTEGER)
            return TypeCode::INTEGER;
        if ((left_type == TypeCode::REAL || left_type == TypeCode::INTEGER) &&
            (right_type == TypeCode::REAL || right_type == TypeCode::INTEGER))
            return TypeCode::REAL;
        // String concatenation tidak didukung secara eksplisit
        return TypeCode::UNKNOWN;
    }

    // Operator pembagian: /
    if (op == "/")
    {
        if ((left_type == TypeCode::INTEGER || left_type == TypeCode::REAL) &&
            (right_type == TypeCode::INTEGER || right_type == TypeCode::REAL))
            return TypeCode::REAL; // pembagian selalu real
        return TypeCode::UNKNOWN;
    }

    // Operator integer: div, mod
    if (op == "div" || op == "mod")
    {
        if (left_type == TypeCode::INTEGER && right_type == TypeCode::INTEGER)
            return TypeCode::INTEGER;
        return TypeCode::UNKNOWN;
    }

    // Operator boolean: and, or
    if (op == "and" || op == "or")
    {
        if (left_type == TypeCode::BOOLEAN && right_type == TypeCode::BOOLEAN)
            return TypeCode::BOOLEAN;
        return TypeCode::UNKNOWN;
    }

    // Operator relasional: ==, <>, <, >, <=, >=
    if (op == "==" || op == "<>" || op == "<" ||
        op == ">"  || op == "<=" || op == ">=")
    {
        // Kedua operand harus compatible
        if (is_compatible(left_type, right_type))
            return TypeCode::BOOLEAN;
        // Integer dan Real bisa dibandingkan
        if ((left_type == TypeCode::INTEGER || left_type == TypeCode::REAL) &&
            (right_type == TypeCode::INTEGER || right_type == TypeCode::REAL))
            return TypeCode::BOOLEAN;
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

// ============================================================
// HELPER — TypeCode to string (untuk pesan error)
// ============================================================

std::string typecode_to_string(TypeCode t)
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