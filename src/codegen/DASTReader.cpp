
#include "DASTReader.hpp"

#include <cstdlib>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "../semantic/ast/DeclNodes.hpp"
#include "../semantic/ast/StmtNodes.hpp"
#include "../semantic/ast/ExprNodes.hpp"

namespace
{
// Glyph yang dipakai TreePrinter pada keluaran semantic.
const std::string PIPE   = "\xE2\x94\x82   ";                 // "|   "
const std::string BLANK  = "    ";
const std::string BRANCH = "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 "; // "|-- "
const std::string LAST   = "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 "; // "`-- "

// ---- Pohon generik hasil fase 1 -------------------------------------------
struct RawNode
{
    std::string type;                 // mis. "VarNode", "AssignNode"
    std::string params;               // teks di dalam "(...)" atau ""
    int tab_index = -1;
    int lev = -1;
    TypeCode type_code = TypeCode::UNKNOWN;
    std::vector<std::pair<std::string, RawNode *>> children; // (role, anak)
};

std::vector<RawNode *> g_pool; // kepemilikan seluruh RawNode (dibebaskan di akhir)

RawNode *new_raw()
{
    RawNode *n = new RawNode();
    g_pool.push_back(n);
    return n;
}

void free_pool()
{
    for (std::size_t i = 0; i < g_pool.size(); ++i)
        delete g_pool[i];
    g_pool.clear();
}

std::string trim(const std::string &s)
{
    std::size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    std::size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

TypeCode parse_typecode(const std::string &raw)
{
    std::string s;
    for (std::size_t i = 0; i < raw.size(); ++i)
        s += static_cast<char>(std::toupper(static_cast<unsigned char>(raw[i])));
    if (s == "INTEGER")  return TypeCode::INTEGER;
    if (s == "REAL")     return TypeCode::REAL;
    if (s == "CHAR")     return TypeCode::CHAR;
    if (s == "BOOLEAN")  return TypeCode::BOOLEAN;
    if (s == "STRING")   return TypeCode::STRING;
    if (s == "ARRAY")    return TypeCode::ARRAY;
    if (s == "RECORD")   return TypeCode::RECORD;
    if (s == "SUBRANGE") return TypeCode::SUBRANGE;
    if (s == "VOID")     return TypeCode::VOID;
    if (s == "ENUM")     return TypeCode::ENUM;
    return TypeCode::UNKNOWN;
}

ObjClass parse_objclass(const std::string &s)
{
    if (s == "CONSTANT")  return ObjClass::CONSTANT;
    if (s == "VARIABLE")  return ObjClass::VARIABLE;
    if (s == "TYPE")      return ObjClass::TYPE;
    if (s == "PROCEDURE") return ObjClass::PROCEDURE;
    if (s == "FUNCTION")  return ObjClass::FUNCTION;
    if (s == "PROGRAM")   return ObjClass::PROGRAM;
    return ObjClass::RESERVED;
}

// Ekstrak nilai bertanda kutip tunggal: key: 'value'
std::string quoted_value(const std::string &params, const std::string &key)
{
    std::string pat = key + ": '";
    std::size_t p = params.find(pat);
    if (p == std::string::npos) return "";
    p += pat.size();
    std::size_t q = params.find('\'', p);
    if (q == std::string::npos) return params.substr(p);
    return params.substr(p, q - p);
}

// Parse anotasi "[tab_index:.., type:.., lev:..]" pada RawNode.
void parse_annotation(RawNode *node, const std::string &annot)
{
    std::stringstream ss(annot);
    std::string item;
    while (std::getline(ss, item, ','))
    {
        std::string t = trim(item);
        std::size_t colon = t.find(':');
        if (colon == std::string::npos) continue;
        std::string key = trim(t.substr(0, colon));
        std::string val = trim(t.substr(colon + 1));
        if (key == "tab_index") node->tab_index = std::atoi(val.c_str());
        else if (key == "lev")  node->lev = std::atoi(val.c_str());
        else if (key == "type") node->type_code = parse_typecode(val);
    }
}

// Parse satu baris konten (tanpa prefix glyph) menjadi role + RawNode.
RawNode *parse_content(const std::string &content, std::string &role_out)
{
    std::string s = content;
    role_out = "";

    // role di awal: "[role] "
    if (!s.empty() && s[0] == '[')
    {
        std::size_t close = s.find("] ");
        if (close != std::string::npos)
        {
            role_out = s.substr(1, close - 1);
            s = s.substr(close + 2);
        }
    }

    RawNode *node = new_raw();

    // Pisahkan anotasi " [..]" di akhir (label tidak pernah memakai '[').
    std::string label = s;
    std::size_t ann = s.find(" [");
    if (ann != std::string::npos && s[s.size() - 1] == ']')
    {
        label = s.substr(0, ann);
        std::string annot = s.substr(ann + 2, s.size() - (ann + 2) - 1);
        parse_annotation(node, annot);
    }

    // Pisahkan tipe node dan params di dalam "(...)".
    std::size_t paren = label.find('(');
    if (paren == std::string::npos)
    {
        node->type = trim(label);
        node->params = "";
    }
    else
    {
        node->type = trim(label.substr(0, paren));
        std::size_t close = label.rfind(')');
        node->params = label.substr(paren + 1,
                                    close == std::string::npos ? std::string::npos
                                                               : close - paren - 1);
    }
    return node;
}

// ---- Fase 1: bangun pohon RawNode dari daftar baris ------------------------
RawNode *build_raw_tree(const std::vector<std::string> &lines)
{
    RawNode *root = nullptr;
    std::vector<std::pair<int, RawNode *>> stack; // (depth, node)

    for (std::size_t li = 0; li < lines.size(); ++li)
    {
        const std::string &line = lines[li];
        if (trim(line).empty()) continue;

        std::size_t pos = 0;
        int depth = 0;
        bool glyph = false;
        while (pos < line.size())
        {
            if (line.compare(pos, PIPE.size(), PIPE) == 0) { pos += PIPE.size(); depth++; }
            else if (line.compare(pos, BLANK.size(), BLANK) == 0) { pos += BLANK.size(); depth++; }
            else if (line.compare(pos, BRANCH.size(), BRANCH) == 0) { pos += BRANCH.size(); glyph = true; break; }
            else if (line.compare(pos, LAST.size(), LAST) == 0) { pos += LAST.size(); glyph = true; break; }
            else break;
        }
        int node_depth = glyph ? depth + 1 : depth;
        std::string content = line.substr(pos);

        std::string role;
        RawNode *node = parse_content(content, role);

        if (node_depth == 0)
        {
            root = node;
            stack.clear();
            stack.push_back(std::make_pair(0, node));
            continue;
        }

        // Cari induk: entri stack dengan depth = node_depth - 1.
        while (!stack.empty() && stack.back().first >= node_depth)
            stack.pop_back();
        if (stack.empty())
            throw std::runtime_error("DASTReader: struktur indentasi tidak valid");
        stack.back().second->children.push_back(std::make_pair(role, node));
        stack.push_back(std::make_pair(node_depth, node));
    }

    if (!root)
        throw std::runtime_error("DASTReader: Decorated AST kosong");
    return root;
}

// ---- Fase 2: konversi RawNode -> ASTNode -----------------------------------
std::unique_ptr<ASTNode> convert(RawNode *r);

void set_annot(ASTNode *n, RawNode *r)
{
    n->tab_index = r->tab_index;
    n->type_code = r->type_code;
    n->lev = r->lev;
}

RawNode *child_role(RawNode *r, const std::string &role)
{
    for (std::size_t i = 0; i < r->children.size(); ++i)
        if (r->children[i].first == role)
            return r->children[i].second;
    return nullptr;
}

std::unique_ptr<ASTNode> convert_child(RawNode *r, const std::string &role)
{
    RawNode *c = child_role(r, role);
    return c ? convert(c) : std::unique_ptr<ASTNode>();
}

// Parse daftar nama bertanda kutip: 'a', 'b', 'c'
std::vector<std::string> parse_name_list(const std::string &params, const std::string &key)
{
    std::vector<std::string> out;
    std::string pat = key + ": ";
    std::size_t p = params.find(pat);
    if (p == std::string::npos) return out;
    p += pat.size();
    std::string rest = params.substr(p);
    std::size_t i = 0;
    while (i < rest.size())
    {
        std::size_t q1 = rest.find('\'', i);
        if (q1 == std::string::npos) break;
        std::size_t q2 = rest.find('\'', q1 + 1);
        if (q2 == std::string::npos) break;
        out.push_back(rest.substr(q1 + 1, q2 - q1 - 1));
        i = q2 + 1;
    }
    return out;
}

std::unique_ptr<ASTNode> convert(RawNode *r)
{
    const std::string &t = r->type;

    if (t == "ProgramNode")
    {
        std::unique_ptr<ProgramNode> n(new ProgramNode());
        n->name = quoted_value(r->params, "name");
        for (std::size_t i = 0; i < r->children.size(); ++i)
        {
            const std::string &role = r->children[i].first;
            if (role == "decl") n->declarations.push_back(convert(r->children[i].second));
            else if (role == "body") n->body = convert(r->children[i].second);
        }
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "VarDeclNode")
    {
        std::unique_ptr<VarDeclNode> n(new VarDeclNode());
        n->names = parse_name_list(r->params, "names");
        n->type_node = convert_child(r, "type");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ConstDeclNode")
    {
        std::unique_ptr<ConstDeclNode> n(new ConstDeclNode());
        n->name = quoted_value(r->params, "name");
        n->value = convert_child(r, "value");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "TypeDeclNode")
    {
        std::unique_ptr<TypeDeclNode> n(new TypeDeclNode());
        n->name = quoted_value(r->params, "name");
        n->type_def = convert_child(r, "type");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ParamGroupNode")
    {
        std::unique_ptr<ParamGroupNode> n(new ParamGroupNode());
        n->is_var_param = (r->params.find("var: true") != std::string::npos);
        n->names = parse_name_list(r->params, "names");
        n->type_node = convert_child(r, "type");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ProcDeclNode")
    {
        std::unique_ptr<ProcDeclNode> n(new ProcDeclNode());
        n->name = quoted_value(r->params, "name");
        for (std::size_t i = 0; i < r->children.size(); ++i)
        {
            const std::string &role = r->children[i].first;
            if (role == "param") n->params.push_back(convert(r->children[i].second));
            else if (role == "body") n->body = convert(r->children[i].second);
        }
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "FuncDeclNode")
    {
        std::unique_ptr<FuncDeclNode> n(new FuncDeclNode());
        n->name = quoted_value(r->params, "name");
        for (std::size_t i = 0; i < r->children.size(); ++i)
        {
            const std::string &role = r->children[i].first;
            if (role == "param") n->params.push_back(convert(r->children[i].second));
            else if (role == "returns") n->return_type = convert(r->children[i].second);
            else if (role == "body") n->body = convert(r->children[i].second);
        }
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ArrayTypeNode")
    {
        std::unique_ptr<ArrayTypeNode> n(new ArrayTypeNode());
        n->index_range = convert_child(r, "index");
        n->elem_type = convert_child(r, "elem");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "RecordTypeNode")
    {
        std::unique_ptr<RecordTypeNode> n(new RecordTypeNode());
        for (std::size_t i = 0; i < r->children.size(); ++i)
            if (r->children[i].first == "field")
                n->fields.push_back(convert(r->children[i].second));
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "SubrangeNode")
    {
        std::unique_ptr<SubrangeNode> n(new SubrangeNode());
        n->low = convert_child(r, "low");
        n->high = convert_child(r, "high");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "EnumNode")
    {
        std::unique_ptr<EnumNode> n(new EnumNode());
        n->values = parse_name_list(r->params, "values");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "BlockNode")
    {
        std::unique_ptr<BlockNode> n(new BlockNode());
        std::string b = quoted_value(r->params, "btab_index");
        // btab_index bukan bertanda kutip; ambil langsung bila ada.
        std::size_t p = r->params.find("btab_index:");
        if (p != std::string::npos)
            n->btab_index = std::atoi(r->params.c_str() + p + 11);
        for (std::size_t i = 0; i < r->children.size(); ++i)
            if (r->children[i].first == "stmt")
                n->statements.push_back(convert(r->children[i].second));
        set_annot(n.get(), r);
        (void)b;
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "AssignNode")
    {
        std::unique_ptr<AssignNode> n(new AssignNode());
        n->target = convert_child(r, "target");
        n->value = convert_child(r, "value");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "IfNode")
    {
        std::unique_ptr<IfNode> n(new IfNode());
        n->condition = convert_child(r, "cond");
        n->then_branch = convert_child(r, "then");
        n->else_branch = convert_child(r, "else");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "WhileNode")
    {
        std::unique_ptr<WhileNode> n(new WhileNode());
        n->condition = convert_child(r, "cond");
        n->body = convert_child(r, "body");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ForNode")
    {
        std::unique_ptr<ForNode> n(new ForNode());
        n->var_name = quoted_value(r->params, "var");
        n->is_downto = (r->params.find("dir: downto") != std::string::npos);
        n->from_expr = convert_child(r, "from");
        n->to_expr = convert_child(r, "to");
        n->body = convert_child(r, "body");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "RepeatNode")
    {
        std::unique_ptr<RepeatNode> n(new RepeatNode());
        for (std::size_t i = 0; i < r->children.size(); ++i)
            if (r->children[i].first == "stmt")
                n->body.push_back(convert(r->children[i].second));
        n->condition = convert_child(r, "until");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "CaseNode")
    {
        std::unique_ptr<CaseNode> n(new CaseNode());
        n->selector = convert_child(r, "selector");
        // Tiap [label] adalah anak CaseNode; statement-nya adalah anak [stmt]
        // dari RawNode label tersebut (lihat CaseNode::print yang meng-indent
        // statement satu tingkat lebih dalam).
        for (std::size_t i = 0; i < r->children.size(); ++i)
        {
            if (r->children[i].first != "label") continue;
            RawNode *labelRaw = r->children[i].second;
            std::unique_ptr<ASTNode> label = convert(labelRaw);
            std::unique_ptr<ASTNode> stmt;
            RawNode *stmtRaw = child_role(labelRaw, "stmt");
            if (stmtRaw) stmt = convert(stmtRaw);
            n->cases.push_back(std::make_pair(std::move(label), std::move(stmt)));
        }
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ProcCallNode")
    {
        std::unique_ptr<ProcCallNode> n(new ProcCallNode());
        n->name = quoted_value(r->params, "name");
        for (std::size_t i = 0; i < r->children.size(); ++i)
            if (r->children[i].first == "arg")
                n->args.push_back(convert(r->children[i].second));
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "BinOpNode")
    {
        std::unique_ptr<BinOpNode> n(new BinOpNode());
        n->op = quoted_value(r->params, "op");
        n->left = convert_child(r, "left");
        n->right = convert_child(r, "right");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "UnaryOpNode")
    {
        std::unique_ptr<UnaryOpNode> n(new UnaryOpNode());
        n->op = quoted_value(r->params, "op");
        n->operand = convert_child(r, "operand");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "VarNode")
    {
        std::unique_ptr<VarNode> n(new VarNode());
        n->name = quoted_value(r->params, "name");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "NumberNode")
    {
        std::unique_ptr<NumberNode> n(new NumberNode());
        std::size_t p = r->params.find("value:");
        n->value = (p != std::string::npos) ? std::atoi(r->params.c_str() + p + 6) : 0;
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "RealNode")
    {
        std::unique_ptr<RealNode> n(new RealNode());
        std::size_t p = r->params.find("value:");
        n->value = (p != std::string::npos) ? std::atof(r->params.c_str() + p + 6) : 0.0;
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "CharNode")
    {
        std::unique_ptr<CharNode> n(new CharNode());
        std::size_t p = r->params.find("value: '");
        n->value = (p != std::string::npos) ? r->params[p + 8] : '\0';
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "StringNode")
    {
        std::unique_ptr<StringNode> n(new StringNode());
        std::size_t p = r->params.find("value: \"");
        if (p != std::string::npos)
        {
            std::size_t start = p + 8;
            std::size_t end = r->params.rfind('"');
            if (end != std::string::npos && end > start)
                n->value = r->params.substr(start, end - start);
        }
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "BoolNode")
    {
        std::unique_ptr<BoolNode> n(new BoolNode());
        n->value = (r->params.find("value: true") != std::string::npos);
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "ArrayAccessNode")
    {
        std::unique_ptr<ArrayAccessNode> n(new ArrayAccessNode());
        n->array = convert_child(r, "array");
        for (std::size_t i = 0; i < r->children.size(); ++i)
            if (r->children[i].first == "index")
                n->indices.push_back(convert(r->children[i].second));
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "RecordAccessNode")
    {
        std::unique_ptr<RecordAccessNode> n(new RecordAccessNode());
        n->field_name = quoted_value(r->params, "field");
        n->record = convert_child(r, "record");
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }
    if (t == "FuncCallExprNode")
    {
        std::unique_ptr<FuncCallExprNode> n(new FuncCallExprNode());
        n->name = quoted_value(r->params, "name");
        for (std::size_t i = 0; i < r->children.size(); ++i)
            if (r->children[i].first == "arg")
                n->args.push_back(convert(r->children[i].second));
        set_annot(n.get(), r);
        return std::unique_ptr<ASTNode>(std::move(n));
    }

    throw std::runtime_error("DASTReader: jenis node tidak dikenal '" + t + "'");
}

// ---- Parsing Symbol Table --------------------------------------------------
std::vector<std::string> tokenize(const std::string &line)
{
    std::vector<std::string> out;
    std::stringstream ss(line);
    std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}

void parse_symbol_table(const std::vector<std::string> &lines, SymbolTable &sym)
{
    std::vector<TabEntry> tab;
    std::vector<BtabEntry> btab;
    std::vector<AtabEntry> atab;

    enum Section { NONE, T, B, A } sec = NONE;

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        std::string line = lines[i];
        std::string t = trim(line);
        if (t.empty()) continue;
        if (t == "TAB") { sec = T; continue; }
        if (t == "BTAB") { sec = B; continue; }
        if (t == "ATAB") { sec = A; continue; }
        // Lewati baris header kolom.
        if (t.compare(0, 3, "idx") == 0) continue;

        std::vector<std::string> tk = tokenize(line);
        if (sec == T && tk.size() >= 9)
        {
            // idx id link obj type ref nrm lev adr
            TabEntry e;
            e.id   = tk[1];
            e.link = std::atoi(tk[2].c_str());
            e.obj  = parse_objclass(tk[3]);
            e.type = parse_typecode(tk[4]);
            e.ref  = std::atoi(tk[5].c_str());
            e.nrm  = std::atoi(tk[6].c_str());
            e.lev  = std::atoi(tk[7].c_str());
            e.adr  = std::atoi(tk[8].c_str());
            tab.push_back(e);
        }
        else if (sec == B && tk.size() >= 5)
        {
            // idx last lpar psze vsze
            BtabEntry e;
            e.last = std::atoi(tk[1].c_str());
            e.lpar = std::atoi(tk[2].c_str());
            e.psze = std::atoi(tk[3].c_str());
            e.vsze = std::atoi(tk[4].c_str());
            btab.push_back(e);
        }
        else if (sec == A && tk.size() >= 8)
        {
            // idx xtyp etyp eref low high elsz size
            AtabEntry e;
            e.xtyp = parse_typecode(tk[1]);
            e.etyp = parse_typecode(tk[2]);
            e.eref = std::atoi(tk[3].c_str());
            e.low  = std::atoi(tk[4].c_str());
            e.high = std::atoi(tk[5].c_str());
            e.elsz = std::atoi(tk[6].c_str());
            e.size = std::atoi(tk[7].c_str());
            atab.push_back(e);
        }
    }

    if (tab.empty() || btab.empty())
        throw std::runtime_error("DASTReader: Symbol Table tidak lengkap");
    sym.load_dump(tab, btab, atab);
}

std::vector<std::string> split_lines(const std::string &text)
{
    std::vector<std::string> out;
    std::string line;
    std::stringstream ss(text);
    while (std::getline(ss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        out.push_back(line);
    }
    return out;
}
} // namespace

std::unique_ptr<ASTNode> load_decorated_ast(const std::string &text, SymbolTable &sym)
{
    std::vector<std::string> all = split_lines(text);

    // Pisahkan bagian Decorated AST dan Symbol Table berdasarkan penanda.
    int ast_start = -1, sym_start = -1;
    for (std::size_t i = 0; i < all.size(); ++i)
    {
        std::string t = trim(all[i]);
        if (t.find("DECORATED AST") != std::string::npos) ast_start = static_cast<int>(i) + 1;
        else if (t.find("SYMBOL TABLE") != std::string::npos) sym_start = static_cast<int>(i) + 1;
    }
    if (sym_start < 0)
        throw std::runtime_error("DASTReader: bagian SYMBOL TABLE tidak ditemukan");
    if (ast_start < 0) ast_start = 0;

    std::vector<std::string> ast_lines(all.begin() + ast_start,
                                       all.begin() + (sym_start - 1));
    std::vector<std::string> sym_lines(all.begin() + sym_start, all.end());

    parse_symbol_table(sym_lines, sym);

    RawNode *root = build_raw_tree(ast_lines);
    std::unique_ptr<ASTNode> ast;
    try
    {
        ast = convert(root);
    }
    catch (...)
    {
        free_pool();
        throw;
    }
    free_pool();
    return ast;
}
