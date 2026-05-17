#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <string>
#include <vector>
#include "../ast/ASTNode.hpp" // untuk TypeCode dan ObjClass

struct TabEntry
{
    std::string id; // nama identifier
    int link;       // index ke tab entry sebelumnya dalam scope yang sama (linked list per blok)
    ObjClass obj;   // kelas objek: CONSTANT, VARIABLE, TYPE, PROCEDURE, FUNCTION
    TypeCode type;  // tipe data identifier
    int ref;        // index ke atab (jika ARRAY) atau btab (jika RECORD/PROCEDURE/FUNCTION)
    int nrm;        // 1 = variabel normal, 0 = var-parameter (by-reference)
    int lev;        // lexical level tempat identifier dideklarasikan (0 = global)
    int adr;        // offset variabel / nilai konstanta / ukuran / alamat prosedur
};

struct BtabEntry
{
    int last; // index tab dari identifier terakhir yang dideklarasikan di blok ini
    int lpar; // index tab dari parameter terakhir (0 jika record / tidak ada parameter)
    int psze; // total ukuran parameter (dalam unit memori)
    int vsze; // total ukuran variabel lokal (dalam unit memori)
};

struct AtabEntry
{
    TypeCode xtyp; // tipe index array
    TypeCode etyp; // tipe elemen array
    int eref;      // index ke atab/btab jika elemen komposit (-1 jika tidak ada)
    int low;       // batas bawah index
    int high;      // batas atas index
    int elsz;      // ukuran satu elemen (dalam unit memori)
    int size;      // total ukuran array = (high - low + 1) * elsz
};

class SymbolTable
{
public:
    SymbolTable();

    //  Menambah entri
    int enter_tab(const TabEntry &e);   // tambah ke tab,  return index baru
    int enter_btab(const BtabEntry &e); // tambah ke btab, return index baru
    int enter_atab(const AtabEntry &e); // tambah ke atab, return index baru

    // Lookup
    // Cari identifier 'name' mulai dari level saat ini ke atas (scope terluar)
    // Return index di tab, atau -1 jika tidak ditemukan
    int lookup(const std::string &name) const;

    int lookup_in_level(const std::string &name, int level) const;

    int open_scope();
    void close_scope();

    TabEntry &get_tab(int i);
    BtabEntry &get_btab(int i);
    AtabEntry &get_atab(int i);

    const TabEntry &get_tab(int i) const;
    const BtabEntry &get_btab(int i) const;
    const AtabEntry &get_atab(int i) const;

    int current_level() const;
    int current_btab_index() const;

    void print_all(std::ostream &out) const;

private:
    std::vector<TabEntry> tab;
    std::vector<BtabEntry> btab;
    std::vector<AtabEntry> atab;

    std::vector<int> scope_stack;

    void init_predefined();
};

#endif