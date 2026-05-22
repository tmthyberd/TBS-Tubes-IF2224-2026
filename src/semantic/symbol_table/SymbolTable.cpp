#include "Symbol_Table.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <stdexcept>

namespace
{
std::string to_lower(const std::string &value)
{
    std::string result = value;
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

std::string type_to_string(TypeCode type)
{
    switch (type)
    {
    case TypeCode::INTEGER: return "INTEGER";
    case TypeCode::REAL: return "REAL";
    case TypeCode::CHAR: return "CHAR";
    case TypeCode::BOOLEAN: return "BOOLEAN";
    case TypeCode::STRING: return "STRING";
    case TypeCode::ARRAY: return "ARRAY";
    case TypeCode::RECORD: return "RECORD";
    case TypeCode::SUBRANGE: return "SUBRANGE";
    case TypeCode::VOID: return "VOID";
    case TypeCode::ENUM: return "ENUM";
    default: return "UNKNOWN";
    }
}

std::string obj_to_string(ObjClass obj)
{
    switch (obj)
    {
    case ObjClass::CONSTANT: return "CONSTANT";
    case ObjClass::VARIABLE: return "VARIABLE";
    case ObjClass::TYPE: return "TYPE";
    case ObjClass::PROCEDURE: return "PROCEDURE";
    case ObjClass::FUNCTION: return "FUNCTION";
    case ObjClass::RESERVED: return "RESERVED";
    case ObjClass::PROGRAM: return "PROGRAM";
    default: return "UNKNOWN";
    }
}

TabEntry make_tab_entry(const std::string &id,
                        ObjClass obj,
                        TypeCode type,
                        int ref = -1,
                        int adr = 0)
{
    TabEntry entry;
    entry.id = id;
    entry.link = -1;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.nrm = 1;
    entry.lev = 0;
    entry.adr = adr;
    return entry;
}
}

// Constructor

SymbolTable::SymbolTable()
{
    init_predefined();
}

// Entry Insertion

int SymbolTable::enter_tab(const TabEntry &e)
{
    TabEntry entry = e;

    if (!scope_stack.empty())
    {
        int current = current_btab_index();
        entry.link = btab[current].last;
        entry.lev = current_level();
    }

    tab.push_back(entry);
    int index = static_cast<int>(tab.size()) - 1;

    if (!scope_stack.empty())
        btab[current_btab_index()].last = index;

    return index;
}

int SymbolTable::enter_btab(const BtabEntry &e)
{
    btab.push_back(e);
    return static_cast<int>(btab.size()) - 1;
}

int SymbolTable::enter_atab(const AtabEntry &e)
{
    atab.push_back(e);
    return static_cast<int>(atab.size()) - 1;
}

// Lookup

int SymbolTable::lookup(const std::string &name) const
{
    std::string target = to_lower(name);

    for (std::vector<int>::const_reverse_iterator it = scope_stack.rbegin();
         it != scope_stack.rend(); ++it)
    {
        int tab_index = btab[*it].last;
        while (tab_index >= 0)
        {
            if (to_lower(tab[tab_index].id) == target)
                return tab_index;
            tab_index = tab[tab_index].link;
        }
    }

    return -1;
}

int SymbolTable::lookup_in_level(const std::string &name, int level) const
{
    std::string target = to_lower(name);

    if (level < 0 || level >= static_cast<int>(scope_stack.size()))
        return -1;

    int tab_index = btab[scope_stack[static_cast<std::size_t>(level)]].last;
    while (tab_index >= 0)
    {
        if (to_lower(tab[tab_index].id) == target)
            return tab_index;
        tab_index = tab[tab_index].link;
    }

    return -1;
}

// Scope Management

int SymbolTable::open_scope()
{
    BtabEntry block;
    block.last = -1;
    block.lpar = -1;
    block.psze = 0;
    block.vsze = 0;

    int index = enter_btab(block);
    scope_stack.push_back(index);
    return index;
}

void SymbolTable::close_scope()
{
    if (scope_stack.size() > 1)
        scope_stack.pop_back();
}

// Accessors

TabEntry &SymbolTable::get_tab(int i)
{
    return tab.at(static_cast<std::size_t>(i));
}

BtabEntry &SymbolTable::get_btab(int i)
{
    return btab.at(static_cast<std::size_t>(i));
}

AtabEntry &SymbolTable::get_atab(int i)
{
    return atab.at(static_cast<std::size_t>(i));
}

const TabEntry &SymbolTable::get_tab(int i) const
{
    return tab.at(static_cast<std::size_t>(i));
}

const BtabEntry &SymbolTable::get_btab(int i) const
{
    return btab.at(static_cast<std::size_t>(i));
}

const AtabEntry &SymbolTable::get_atab(int i) const
{
    return atab.at(static_cast<std::size_t>(i));
}

int SymbolTable::current_level() const
{
    if (scope_stack.empty())
        return -1;
    return static_cast<int>(scope_stack.size()) - 1;
}

int SymbolTable::current_btab_index() const
{
    if (scope_stack.empty())
        return -1;
    return scope_stack.back();
}

// Output

void SymbolTable::print_all(std::ostream &out) const
{
    out << "TAB\n";
    out << "idx id                  link obj        type       ref nrm lev adr\n";
    for (std::size_t i = 0; i < tab.size(); ++i)
    {
        out << std::setw(3) << i << " "
            << std::setw(19) << std::left << tab[i].id << std::right << " "
            << std::setw(4) << tab[i].link << " "
            << std::setw(10) << std::left << obj_to_string(tab[i].obj) << std::right << " "
            << std::setw(10) << std::left << type_to_string(tab[i].type) << std::right << " "
            << std::setw(3) << tab[i].ref << " "
            << std::setw(3) << tab[i].nrm << " "
            << std::setw(3) << tab[i].lev << " "
            << std::setw(3) << tab[i].adr << "\n";
    }

    out << "\nBTAB\n";
    out << "idx last lpar psze vsze\n";
    for (std::size_t i = 0; i < btab.size(); ++i)
    {
        out << std::setw(3) << i << " "
            << std::setw(4) << btab[i].last << " "
            << std::setw(4) << btab[i].lpar << " "
            << std::setw(4) << btab[i].psze << " "
            << std::setw(4) << btab[i].vsze << "\n";
    }

    out << "\nATAB\n";
    out << "idx xtyp       etyp       eref low high elsz size\n";
    for (std::size_t i = 0; i < atab.size(); ++i)
    {
        out << std::setw(3) << i << " "
            << std::setw(10) << std::left << type_to_string(atab[i].xtyp) << std::right << " "
            << std::setw(10) << std::left << type_to_string(atab[i].etyp) << std::right << " "
            << std::setw(4) << atab[i].eref << " "
            << std::setw(3) << atab[i].low << " "
            << std::setw(4) << atab[i].high << " "
            << std::setw(4) << atab[i].elsz << " "
            << std::setw(4) << atab[i].size << "\n";
    }
}

// Predefined Symbols

void SymbolTable::init_predefined()
{
    scope_stack.clear();
    tab.clear();
    btab.clear();
    atab.clear();

    open_scope();

    enter_tab(make_tab_entry("and", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("array", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("begin", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("case", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("const", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("div", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("downto", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("do", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("else", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("end", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("for", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("function", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("if", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("mod", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("not", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("of", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("or", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("procedure", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("program", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("record", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("repeat", ObjClass::RESERVED, TypeCode::VOID));

    enter_tab(make_tab_entry("integer", ObjClass::TYPE, TypeCode::INTEGER, -1, 1));
    enter_tab(make_tab_entry("real", ObjClass::TYPE, TypeCode::REAL, -1, 1));
    enter_tab(make_tab_entry("boolean", ObjClass::TYPE, TypeCode::BOOLEAN, -1, 1));
    enter_tab(make_tab_entry("char", ObjClass::TYPE, TypeCode::CHAR, -1, 1));
    enter_tab(make_tab_entry("string", ObjClass::TYPE, TypeCode::STRING, -1, 1));

    enter_tab(make_tab_entry("then", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("to", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("type", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("until", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("var", ObjClass::RESERVED, TypeCode::VOID));
    enter_tab(make_tab_entry("while", ObjClass::RESERVED, TypeCode::VOID));

    enter_tab(make_tab_entry("true", ObjClass::CONSTANT, TypeCode::BOOLEAN, -1, 1));
    enter_tab(make_tab_entry("false", ObjClass::CONSTANT, TypeCode::BOOLEAN, -1, 0));

    enter_tab(make_tab_entry("writeln", ObjClass::PROCEDURE, TypeCode::VOID));
    enter_tab(make_tab_entry("readln", ObjClass::PROCEDURE, TypeCode::VOID));
    enter_tab(make_tab_entry("write", ObjClass::PROCEDURE, TypeCode::VOID));
    enter_tab(make_tab_entry("read", ObjClass::PROCEDURE, TypeCode::VOID));
}
