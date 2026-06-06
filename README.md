# Lexer Bahasa Pemrograman Arion

**IF2224 - Teori Bahasa Formal dan Automata**  
**Milestone 1: Lexical Analyzer**

## Deskripsi

Implementasi Lexical Analyzer (Lexer) untuk bahasa pemrograman Arion menggunakan prinsip **Deterministic Finite Automata (DFA)**. Lexer ini membaca source code Arion dan menghasilkan daftar token yang dapat digunakan untuk proses parsing selanjutnya.

## Fitur

- Mengenali **52 token** bahasa Arion
- Implementasi berbasis **DFA** (membaca karakter satu per satu, tanpa backtracking)
- Mendukung komentar multiline (`{ }` dan `(* *)`)
- Case-insensitive untuk semua keyword
- Error handling untuk token tidak dikenal dan literal tidak tertutup
- Output terformat ke stdout

## Struktur Direktori

```
├── src/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── token.h              # Definisi TokenType dan Token
│   │   ├── lexer.h              # Deklarasi class Lexer
│   │   └── lexer.cpp            # Implementasi Lexer (DFA)
│   └── parser/
│       └── core/
│           ├── TokenStream.hpp
│           ├── TokenStream.cpp
│           ├── ParseTreeNode.hpp
│           ├── ParseTreeNode.cpp
│           ├── TreePrinter.hpp
│           ├── TreePrinter.cpp
│           ├── ErrorHandler.hpp
│           └── ErrorHandler.cpp
├── test/
│   └── lexer/
│       ├── input-1.txt
│       ├── output-1.txt
│       └── ...
├── doc/
│   └── Laporan-1-TBS.pdf
├── Makefile
└── README.md
```

## Build

```bash
make
```

Target Makefile yang tersedia:

```bash
make runlexer      # build dan jalankan lexer dengan test/lexer/input-1.txt
make run-lexer     # alias dengan tanda hubung
make runparser     # placeholder sampai executable parser tersedia
make run-parser    # alias dengan tanda hubung
make clean         # hapus hasil kompilasi
```

## Penggunaan

```bash
./lexer <input_file.txt> [output_file.txt]
```

Contoh:
```bash
./lexer test/lexer/input-1.txt
./lexer test/lexer/input-1.txt test/lexer/output-1.txt
```

## Daftar Token

| No | Token | Simbol/Kata Kunci |
|----|-------|-------------------|
| 1 | `intcon` | Konstanta integer: `1`, `3`, `48`, `0123` |
| 2 | `realcon` | Konstanta real: `3.14`, `26.7` |
| 3 | `charcon` | Karakter tunggal: `'j'`, `'k'` |
| 4 | `string` | String: `'IRK'`, `'TBFO'`, `''` |
| 5 | `notsy` | `NOT` |
| 6 | `plus` | `+` |
| 7 | `minus` | `-` |
| 8 | `times` | `*` |
| 9 | `idiv` | `div` |
| 10 | `rdiv` | `/` |
| 11 | `imod` | `MOD` |
| 12 | `andsy` | `AND` |
| 13 | `orsy` | `OR` |
| 14 | `eql` | `==` |
| 15 | `neq` | `<>` |
| 16 | `gtr` | `>` |
| 17 | `geq` | `>=` |
| 18 | `lss` | `<` |
| 19 | `leq` | `<=` |
| 20 | `lparent` | `(` |
| 21 | `rparent` | `)` |
| 22 | `lbrack` | `[` |
| 23 | `rbrack` | `]` |
| 24 | `comma` | `,` |
| 25 | `semicolon` | `;` |
| 26 | `period` | `.` |
| 27 | `colon` | `:` |
| 28 | `becomes` | `:=` |
| 29 | `constsy` | `const` |
| 30 | `typesy` | `type` |
| 31 | `varsy` | `var` |
| 32 | `functionsy` | `function` |
| 33 | `proceduresy` | `procedure` |
| 34 | `arraysy` | `array` |
| 35 | `recordsy` | `record` |
| 36 | `programsy` | `program` |
| 37 | `ident` | Identifier: `x`, `PI`, `MyInt` |
| 38 | `beginsy` | `begin` |
| 39 | `ifsy` | `if` |
| 40 | `casesy` | `case` |
| 41 | `repeatsy` | `repeat` |
| 42 | `whilesy` | `while` |
| 43 | `forsy` | `for` |
| 44 | `endsy` | `end` |
| 45 | `elsesy` | `else` |
| 46 | `untilsy` | `until` |
| 47 | `ofsy` | `of` |
| 48 | `dosy` | `do` |
| 49 | `tosy` | `to` |
| 50 | `downtosy` | `downto` |
| 51 | `thensy` | `then` |
| 52 | `comment` | `{ ... }` atau `(* ... *)` |

## Catatan Implementasi

### Comment
Implementasi **strict**: pembuka `{` harus ditutup dengan `}`, dan `(*` harus ditutup dengan `*)`. Tidak boleh campur.

### Case-Insensitivity
Semua keyword bersifat case-insensitive. `IF`, `If`, `iF`, `if` semuanya valid.

### Escape Character
Di dalam string/charcon, petik tunggal di-escape dengan menulisnya dua kali: `''`
- `'Baca ''KBBI'''` → `string('Baca 'KBBI'')`
- `''''` → `charcon(')`

### Error Handling
- Token tidak dikenal → `unknown (karakter)`
- String/comment tidak tertutup → `error (pesan)`
- Program tidak crash saat menemui error

## Contoh

Input (`program.txt`):
```pascal
program Hello;
begin
  x := 5;
end.
```

Output:
```
programsy
ident (Hello)
semicolon
beginsy
ident (x)
becomes
intcon (5)
semicolon
endsy
period
```

## Anggota Kelompok

| Nama | NIM |
|------|-----|
| Tengku Naufal Saqib | 13524012 |
| Fayyaz Akmal Lauda | 13524076 |
| Fahd Muhammad Zahid | 13524078 |
| Timothy Bernard Soeharto | 13524092 |

## Pembagian Tugas

| NIM       | Deskripsi Tugas                                                                 | Persentase |
|-----------|--------------------------------------------------------------------------------|------------|
| 13524012  | Membuat DFA, membuat kerangka program, membuat laporan bagian 3.2, membuat program utama                      | 25%        |
| 13524076  | Membuat DFA, memperbaiki program, membuat laporan bagian 1.4, 2.1, merapihkan laporan              | 25%        |
| 13524078  | Membuat DFA, membuat template laporan, laporan bagian 1.3, 2.1, Bab 3, dan Bab 4 | 25%        |
| 13524092  | Membuat DFA, merevisi DFA berdasarkan QnA, pewarnaan DFA, membuat laporan bagian 1.1 dan 1.2 | 25%        |

# Milestone 2 - Syntax Analyzer (Parser)

## Deskripsi
Implementasi **recursive‑descent parser** untuk bahasa Arion sesuai grammar yang telah didefinisikan pada _Laporan‑2‑TBS.md_. Parser membaca file token yang dihasilkan oleh lexer dan membangun **Parse Tree** (`ParseTreeNode`). 

## Build & Run
```bash
make            # membangun lexer & parser
./parser <file_token.txt>      # jalankan parser pada token file
```

## Testing
Folder `test/parser/` berisi lima test case (`input-tc1.txt` ‑ `input-tc5.txt`).  Setiap test case memiliki folder:
- `input/`   - token file input
- `output/`  - hasil sebenarnya setelah menjalankan `./parser`



---

# Milestone 3 - Semantic Analyzer

## Deskripsi
Implementasi **Semantic Analyzer** untuk bahasa Arion. Tahap ini membaca **Parse Tree** keluaran parser, mengubahnya menjadi **Abstract Syntax Tree (AST)**, lalu melakukan pemeriksaan semantik (type checking, scope, deklarasi, kompatibilitas operator) sambil membangun **Symbol Table** (tab/btab/atab) mengikuti model Wirth's Pascal-S.

Keluaran program berupa:
1. **Decorated AST** - AST yang setiap node-nya telah dianotasi `tab_index`, `type`, dan `lev` (lexical level). Dicetak dalam format tree dengan glyph `├──`, `└──`, `│`.
2. **Symbol Table** - tiga tabel paralel:
   - `tab`   - identifier (id, link, obj, type, ref, nrm, lev, adr)
   - `btab`  - block scope (last, lpar, psze, vsze)
   - `atab`  - array (xtyp, etyp, eref, low, high, elsz, size)

## Pipeline
```
source.pas  →  [lexer]  →  tokens  →  [parser]  →  parse-tree  →  [semantic]  →  Decorated AST + Symbol Table
```

## Build & Run
```bash
make            # membangun lexer, parser, dan semantic
./semantic <parse_tree.txt>                   # cetak hasil ke stdout
./semantic <parse_tree.txt> <output.txt>      # plus tulis ke file
```

## Fitur Pemeriksaan Semantik

**Deklarasi & Scope**
- `var`, `const`, `type`, `procedure`, `function` (nested) dengan multi-level scope
- Deteksi *redeclaration* di scope yang sama
- Predefined symbol (reserved keyword + tipe dasar + `true`/`false` + `writeln`/`readln`/`write`/`read`)

**Tipe**
- Primitif: `integer`, `real`, `char`, `boolean`, `string`
- Komposit: `array [low..high] of T`, `record … end`, `subrange`, `enum`
- Type compatibility rules (mis. `integer` promote ke `real` pada operasi mixed)

**Statement & Ekspresi**
- `assignment` - cek kompatibilitas tipe target dengan value nya
- `if` / `while` / `repeat-until` - kondisi wajib `boolean`
- `for` - variabel wajib ordinal (`integer`/`char`)
- `case` - selector wajib ordinal
- `procedure`/`function call` - validasi nama & jenis (proc vs func)
- BinOp `+ - * / div mod and or = <> < > <= >=` dan UnaryOp `- not`
- Akses `array[i]` (index harus kompatibel, tidak boleh `real`) dan `record.field`

**Error Detection (12+ aturan)**
| Aturan | Pesan |
|---|---|
| Identifier ganda di scope yang sama | `Identifier already declared in this scope: 'x'` |
| Identifier tidak dideklarasikan | `Undeclared identifier: 'foo'` |
| Tipe assignment tidak cocok | `Assignment type mismatch: cannot assign X to Y` |
| Kondisi `if`/`while`/`repeat` bukan `boolean` | `If condition must be Boolean, got Integer` |
| Variabel `for` bukan ordinal | `For loop variable must be Integer or Char, got Boolean` |
| `not` bukan `boolean` | `'not' requires Boolean operand, got Integer` |
| Unary `-` bukan numerik | `Unary '-' requires Integer or Real, got Boolean` |
| Subrange `low > high` | `Subrange lower bound must not exceed upper bound` |
| Index array bertipe `real` | `Array index cannot be Real` |
| Pemanggilan non-procedure/function | `'X' is not a procedure` / `… function` |
| Akses field record yang tidak ada | `Record has no field 'X'` |
| Tipe selector `case` non-ordinal | `Case selector must be ordinal …` |

## Struktur Direktori Modul Semantik
```
src/semantic/
├── ASTBuilder.hpp / .cpp            # Transisi ParseTree ke AST
├── ast/
│   ├── ASTNode.hpp                  # base + TypeCode, ObjClass
│   ├── DeclNodes.hpp                # ProgramNode, VarDecl, ConstDecl, ProcDecl, FuncDecl, …
│   ├── StmtNodes.hpp                # Block, Assign, If, While, For, Repeat, Case, ProcCall
│   └── ExprNodes.hpp                # BinOp, Unary, Var, Number, Real, Char, String, Bool, …
├── symbol_table/
│   ├── Symbol_Table.hpp             # TabEntry, BtabEntry, AtabEntry
│   └── SymbolTable.cpp              # init_predefined, enter/lookup, open/close scope, print_all
├── visitor/
│   ├── SemanticVisitor.hpp
│   └── VisitorStatement.cpp         # visit_* untuk seluruh node + helper type checking
└── error/
    └── SemanticError.hpp            # exception class
src/semantic_main.cpp                # entry point: load parse-tree, build AST, visit, print
```

## Testing
Folder `test/semantic/` berisi **7 test case** (`input-tc1.txt` - `input-tc7.txt`):
- `input/`  - parse tree input (keluaran parser)
- `output/` - hasil semantic analysis (Decorated AST + Symbol Table, atau pesan `Semantic Error`)

Cakupan test case:
| TC  | Skenario |
|-----|----------|
| tc1 | Var integer, assignment, binop `+`, pemanggilan `writeln` |
| tc2 | Const real, var real, aritmetika `*` |
| tc3 | **Error:** assignment type mismatch (`String` ke `Integer`) |
| tc4 | `for … to` loop dengan akumulasi |
| tc5 | **Error:** undeclared identifier |
| tc6 | `type record`, `var record`/`array`, `RecordAccess`, `ArrayAccess`, `Subrange`, `for` |
| tc7 | Deklarasi `procedure` & `function`, value-parameter, `FuncCallExpr` |
| tc8 | **Error:** redeclared identifier di scope yang sama (`var x: integer; x: real;`) |

**Hasil:** 8/8 PASS.

## Format Output

**Contoh Decorated AST** (potongan tc1):
```
ProgramNode(name: 'Hello') [type:void, lev:0]
├── [decl] VarDeclNode(names: 'a', 'b') [type:void, lev:0]
│   └── [type] VarNode(name: 'integer') [tab_index:21, type:integer, lev:0]
└── [body] BlockNode(btab_index: 1) [type:void, lev:1]
    ├── [stmt] AssignNode [type:void, lev:1]
    │   ├── [target] VarNode(name: 'a') [tab_index:39, type:integer, lev:0]
    │   └── [value] NumberNode(value: 5) [type:integer, lev:1]
    ...
```

**Contoh Symbol Table** (potongan):
```
TAB
idx id                  link obj        type       ref nrm lev adr
  0 and                   -1 RESERVED   VOID        -1   1   0   0
  ...
 21 integer               20 TYPE       INTEGER     -1   1   0   1
 ...
 38 Hello                 37 PROGRAM    VOID        -1   1   0   0
 39 a                     38 VARIABLE   INTEGER     -1   1   0   0
 40 b                     39 VARIABLE   INTEGER     -1   1   0   1

BTAB
idx last lpar psze vsze
  0   37   -1    0    0
  1   42   -1    0    2

ATAB
idx xtyp       etyp       eref low high elsz size
```

## Pembagian Tugas Milestone 3

| NIM      | Deskripsi Tugas                                                              | Persentase |
|----------|------------------------------------------------------------------------------|------------|
| 13524012 | Implementasi sebagian VisitorStatement dan test case, dan fix main program   | 25%        |
| 13524076 | Implementasi sebagian VisitorStatement, semantic_main, mengisi laporan Bab II| 25%        |
| 13524078 | Implementasi Interface, membuat template dokumen, dan mengisi Bab I          | 25%        |
| 13524092 | Implementasi ASTBuilder dan ASTNode, bantu dokumen                           | 25%        |

---

# Milestone 4 - Intermediate Code Generator & Interpreter

## Deskripsi
Tahap terakhir pipeline kompilator bahasa Arion. Terdiri dari dua komponen:

1. **Intermediate Code Generator** - mengubah **Decorated AST** (keluaran tahap semantik) menjadi daftar instruksi **stack machine** (Intermediate Code) menggunakan penelusuran berbasis **DFS** terhadap AST + teknik **backpatching** untuk control flow.
2. **Interpreter / Virtual Machine** - mengeksekusi Intermediate Code pada sebuah **stack machine** dengan siklus *Fetch-Decode-Execute*, mengelola memori melalui **stack frame** (static link, dynamic link, return address).

Disediakan dua *executable*:

- **`compiler`** - pipeline penuh dari **source code** Arion: `lexer -> parser -> semantic -> codegen -> VM`, menampilkan Intermediate Code **dan** output eksekusi.
- **`codegen`** - masukan berupa **Decorated AST** (keluaran `semantic`), keluaran **hanya Intermediate Code** (tanpa eksekusi). Code generator murni.

## Build & Run
```bash
make                 # build lexer, parser, semantic, compiler, codegen
make compiler        # build compiler (pipeline penuh source -> output)
make codegen         # build code generator (masukan Decorated AST)

# Pipeline penuh dari source Arion:
./compiler <source.txt>
./compiler <source.txt> <output.txt>

# Code generator dengan masukan Decorated AST:
#   (Decorated AST dihasilkan dari source via lexer -> parser -> semantic)
./codegen <decorated_ast.txt>
./codegen <decorated_ast.txt> <output.txt>
```
Pada Windows (MinGW) gunakan `mingw32-make` dan tambahkan akhiran `.exe` (mis. `compiler.exe`).

Target *run* yang tersedia: `make run-compiler`, `make run-codegen`.

## Set Instruksi (Stack Machine)
| OpCode | Keterangan | | OprCode | Operasi |
|---|---|---|---|---|
| `LIT v`   | push literal v          | | 1 `NEG`  | negasi |
| `LOD l,a` | muat nilai dari (l,a)   | | 2 `ADD` / 3 `SUB` | tambah / kurang |
| `STO l,a` | simpan ke (l,a)         | | 4 `MUL` / 5 `DIV` | kali / bagi real |
| `LDA l,a` | push alamat absolut     | | 6 `MOD` / 17 `IDIV` | modulo / `div` |
| `LDI`/`STI` | load/store indirect (array/record) | | 7-12 | `EQL NEQ LSS GEQ GTR LEQ` |
| `CAL l,a` | panggil subprogram      | | 13 `WRT` / 14 `WRTLN` | tulis / tulis+newline |
| `INT m`   | alokasi frame ukuran m  | | 15 `AND` / 16 `OR` | boolean |
| `JMP a` / `JPC a` | lompat / lompat bila false | | | |
| `RET`     | kembali dari subprogram | | | |

## Fitur
- Ekspresi aritmetika, relasional, boolean (presedensi terjaga lewat DFS)
- Control flow: `if-else`, `while`, `for` (`to`/`downto`), `repeat-until`, `case`
- **Array** dan **record** (baca/tulis) via indirect addressing + validasi batas
- **Procedure** dan **function** penuh: parameter nilai, nilai balik (via nama fungsi), **rekursi**, akses variabel global (static link)
- `writeln`/`write` multi-argumen
- **Penanganan runtime error / vulnerabilities:** integer overflow, pembagian/modulo nol, out-of-bounds array, target lompatan invalid, stack overflow/underflow, type mismatch — semua dilaporkan tanpa membuat program *crash*

## Struktur Direktori Modul Milestone 4
```
src/
├── codegen/
│   ├── Instruction.hpp         # OpCode, OprCode, struct Instruction
│   ├── CodegenVisitor.hpp      # antarmuka code generator
│   ├── CodegenExpr.cpp         # codegen ekspresi + OPR mapper
│   ├── CodegenStmt.cpp         # codegen statement, control flow, subprogram
│   └── DASTReader.hpp / .cpp   # pembaca Decorated AST teks ke AST + Symbol Table
├── interpreter/
│   ├── VM_Value.hpp            # runtime value (int/real/char/bool/string)
│   ├── VM_Memory.hpp / .cpp    # slot memori, stack, frame, addressing
│   ├── VirtualMachine.hpp      # antarmuka VM
│   ├── VM_Operations.cpp       # run/step/execute + dispatch instruksi
│   └── VM_Exceptions.hpp / .cpp# hierarki exception runtime
├── compiler_main.cpp           # entry point pipeline penuh (source ke output)
└── codegen_main.cpp            # entry point code generator (Decorated AST ke IC)
```

## Testing
| Folder | Cakupan | Hasil |
|--------|---------|-------|
| `test/codegen/decorated-ast/` | Masukan Decorated AST untuk `codegen` (14 kasus) | 14/14 |
| `test/integration/`    | Pipeline penuh source menuju output (7 kasus) | 7/7 |
| `test/interpreter/`    | Unit instruksi VM + bonus runtime vulnerabilities (10 kasus) | 10/10 |
| `test/semantic/`       | Regresi analisis semantik Milestone 3 (8 kasus) | 8/8 |

## Pembagian Tugas

| NIM | Deskripsi Tugas | Persentase |
|------|----------------|------------|
| 13524012 | Laporan *test case*, implementasi VM Operations, `codegen_main`, interpreter, pengujian (*testing*), serta Codegen Statement | 25% |
| 13524076 | Melengkapi laporan, mengimplementasikan Codegen Expression dan OPR Mapper | 25% |
| 13524078 | Membuat template laporan, menyusun Bab 1, serta menyiapkan *test case* untuk pengujian | 25% |
| 13524092 | Menyusun Bab 2 laporan, mengembangkan fondasi backend compiler meliputi antarmuka code generator, format instruksi, memori VM, representasi nilai runtime, antarmuka VM, dan exception runtime | 25% |
