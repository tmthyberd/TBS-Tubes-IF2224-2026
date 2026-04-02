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
│   ├── token.h       # Definisi TokenType dan Token
│   ├── lexer.h       # Deklarasi class Lexer
│   ├── lexer.cpp     # Implementasi Lexer (DFA)
│   ├── main.cpp      # Entry point
│   └── Makefile      # Build system
├── test/
│   └── milestone-1/
│       ├── input-1.txt
│       ├── output-1.txt
│       └── ...
├── doc/
│   └── Laporan-1-*.pdf
└── README.md
```

## Build

```bash
make
```

## Penggunaan

```bash
./lexer <input_file.txt> [output_file.txt]
```

Contoh:
```bash
./lexer test/milestone-1/input-1.txt
./lexer test/milestone-1/input-1.txt test/milestone-1/output-1.txt
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
