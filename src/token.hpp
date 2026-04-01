#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

/**
 * KERANGKA (SKELETON) - Token Types untuk Pascal Lexer
 * 
 * PETUNJUK IMPLEMENTASI:
 * - Tambahkan enum values untuk setiap jenis token yang diperlukan
 * - Lihat referensi: literals, operators, delimiters, keywords
 */

enum class TokenType {
    // =============================================
    // LITERALS - Sudah diimplementasi sebagai contoh
    // =============================================
    INTCON,      // integer constant: 123, 456
    REALCON,     // real constant: 3.14, 2.5e10
    CHARCON,     // character constant: 'a', 'b'
    STRING,      // string literal: 'hello world'

    // =============================================
    // OPERATORS - TODO: Implementasi sisanya
    // =============================================
    // Contoh yang sudah ada:
    NOTSY,       // not
    PLUS,        // +
    MINUS,       // -
    
    // TODO: Tambahkan operator lainnya seperti:
    // TIMES,    // *
    // IDIV,     // div (integer division)
    // RDIV,     // / (real division)
    // IMOD,     // mod
    // ANDSY,    // and
    // ORSY,     // or
    // EQL,      // =
    // NEQ,      // <>
    // GTR,      // >
    // GEQ,      // >=
    // LSS,      // <
    // LEQ,      // <=
    // BECOMES,  // :=

    // =============================================
    // DELIMITERS - TODO: Implementasi sisanya
    // =============================================
    // Contoh yang sudah ada:
    LPARENT,     // (
    RPARENT,     // )
    
    // TODO: Tambahkan delimiter lainnya seperti:
    // LBRACK,    // [
    // RBRACK,    // ]
    // COMMA,     // ,
    // SEMICOLON, // ;
    // PERIOD,    // .
    // COLON,     // :

    // =============================================
    // KEYWORDS - TODO: Implementasi sisanya
    // =============================================
    // Contoh yang sudah ada:
    CONSTSY,     // const
    VARSY,       // var
    BEGINSY,     // begin
    ENDSY,       // end
    IFSY,        // if
    
    // TODO: Tambahkan keyword lainnya seperti:
    // TYPESY,       // type
    // FUNCTIONSY,   // function
    // PROCEDURESY,  // procedure
    // ARRAYSY,      // array
    // RECORDSY,     // record
    // PROGRAMSY,    // program
    // CASESY,       // case
    // REPEATSY,     // repeat
    // WHILESY,      // while
    // FORSY,        // for
    // ELSESY,       // else
    // UNTILSY,      // until
    // OFSY,         // of
    // DOSY,         // do
    // TOSY,         // to
    // DOWNTOSY,     // downto
    // THENSY,       // then

    // =============================================
    // GENERIC - Sudah lengkap
    // =============================================
    IDENT,       // identifier: myVar, counter, etc.
    COMMENT,     // komentar { ... } atau (* ... *)
    END_OF_FILE, // akhir file
    LEX_ERROR    // error token
};

/**
 * Struktur Token
 * Menyimpan informasi tentang sebuah token
 */
struct Token {
    TokenType type;       // jenis token
    std::string lexeme;   // teks asli dari source code
    int line;             // baris ke-
    int column;           // kolom ke-

    // Constructor dengan default values
    Token(
        TokenType t = TokenType::LEX_ERROR,
        const std::string& l = "",
        int ln = 1,
        int col = 1
    ) : type(t), lexeme(l), line(ln), column(col) {}
};

/**
 * Mengkonversi TokenType ke string untuk output
 * @param type - TokenType yang akan dikonversi
 * @return string representasi dari token type
 */
std::string tokenTypeToString(TokenType type);

/**
 * Mengkonversi Token ke string untuk output
 * Format: "tokentype" atau "tokentype (lexeme)"
 * @param token - Token yang akan dikonversi
 * @return string representasi dari token
 */
std::string tokenToOutputString(const Token& token);

#endif
