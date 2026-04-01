#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

/**
 * KERANGKA (SKELETON) - Lexer Header untuk Pascal Lexer
 * 
 * PETUNJUK IMPLEMENTASI:
 * - State enum berisi semua state DFA untuk mengenali keywords
 * - Setiap keyword memiliki jalur state tersendiri
 * - FINAL state menandakan keyword terdeteksi
 */

class Lexer {
private:
    std::string source;  // source code input
    size_t pos;          // posisi karakter saat ini
    int line;            // nomor baris saat ini
    int column;          // nomor kolom saat ini

    /**
     * DFA States untuk mengenali keyword dan identifier
     * 
     * Pola penamaan:
     * - Huruf tunggal/gabungan = state intermediate (A, AR, ARR, dll)
     * - _FINAL = state akhir yang menerima (ARRAY_FINAL, BEGIN_FINAL, dll)
     */
    enum class State {
        START,           // state awal

        // =============================================
        // DFA KEYWORD PATHS - Contoh implementasi
        // =============================================
        
        // Path untuk "array" dan "and"
        A, AR, ARR, ARRA, ARRAY_FINAL,
        AN, AND_FINAL,

        // Path untuk "begin"
        B, BE, BEG, BEGI, BEGIN_FINAL,

        // Path untuk "case" dan "const"
        C, CA, CAS, CASE_FINAL,
        CO, CON, CONS, CONST_FINAL,

        // TODO: Tambahkan path untuk keyword lainnya:
        // 
        // Path untuk "div", "do", "downto"
        // D, DI, DIV_FINAL,
        // DO_FINAL,
        // DOW, DOWN, DOWNT, DOWNTO_FINAL,
        //
        // Path untuk "else", "end"
        // E, EL, ELS, ELSE_FINAL,
        // EN, END_FINAL,
        //
        // Path untuk "for", "function"
        // F, FO, FOR_FINAL,
        // FU, FUN, FUNC, FUNCT, FUNCTI, FUNCTIO, FUNCTION_FINAL,
        //
        // Path untuk "if"
        // I, IF_FINAL,
        //
        // Path untuk "mod"
        // M, MO, MOD_FINAL,
        //
        // Path untuk "not"
        // N, NO, NOT_FINAL,
        //
        // Path untuk "of", "or"
        // O, OF_FINAL, OR_FINAL,
        //
        // Path untuk "procedure", "program"
        // P, PR, PRO, PROC, PROCE, PROCED, PROCEDU, PROCEDUR, PROCEDURE_FINAL,
        // PROG, PROGR, PROGRA, PROGRAM_FINAL,
        //
        // Path untuk "record", "repeat"
        // R, RE, REC, RECO, RECOR, RECORD_FINAL,
        // REP, REPE, REPEA, REPEAT_FINAL,
        //
        // Path untuk "then", "to", "type"
        // T, TH, THE, THEN_FINAL,
        // TO_FINAL,
        // TY, TYP, TYPE_FINAL,
        //
        // Path untuk "until"
        // U, UN, UNT, UNTI, UNTIL_FINAL,
        //
        // Path untuk "var"
        // V, VA, VAR_FINAL,
        //
        // Path untuk "while"
        // W, WH, WHI, WHIL, WHILE_FINAL,

        IDENT,           // identifier (bukan keyword)
        ERROR_STATE      // error state
    };

public:
    /**
     * Constructor
     * @param input - source code yang akan di-tokenize
     */
    explicit Lexer(const std::string& input);

    /**
     * Tokenize seluruh source code
     * @param includeComments - apakah komentar dimasukkan ke hasil
     * @return vector berisi semua tokens
     */
    std::vector<Token> tokenize(bool includeComments = false);
    
    /**
     * Baca dan return token berikutnya
     * @return Token berikutnya dari source
     */
    Token nextToken();

private:
    // =============================================
    // BASIC CURSOR UTILITIES
    // =============================================
    
    /** Cek apakah sudah di akhir source */
    bool isAtEnd() const;
    
    /** Lihat karakter saat ini tanpa advance */
    char peek() const;
    
    /** Lihat karakter berikutnya tanpa advance */
    char peekNext() const;
    
    /** Maju satu karakter dan return karakter tersebut */
    char advance();
    
    /** Mundur satu karakter */
    void retreatOne();
    
    /** Match karakter tertentu, advance jika cocok */
    bool match(char expected);

    // =============================================
    // CHARACTER CLASSIFICATION
    // =============================================
    
    /** Cek apakah karakter adalah huruf */
    bool isLetter(char c) const;
    
    /** Cek apakah karakter adalah digit */
    bool isDigit(char c) const;
    
    /** Cek apakah karakter adalah alphanumeric */
    bool isAlnum(char c) const;
    
    /** Convert ke lowercase */
    char toLowerChar(char c) const;

    // =============================================
    // WHITESPACE HANDLING
    // =============================================
    
    /** Skip semua whitespace (space, tab, newline) */
    void skipWhitespace();

    // =============================================
    // MAIN TOKEN READERS - TODO: Implementasi
    // =============================================
    
    /**
     * Baca token keyword atau identifier
     * Menggunakan DFA untuk membedakan keyword dari identifier
     */
    Token readWordLikeToken();
    
    /**
     * Baca token angka (INTCON atau REALCON)
     * Format: 123, 3.14, 2.5e10, dll
     */
    Token readNumberToken();
    
    /**
     * Baca token string atau character
     * Format: 'hello', 'a'
     */
    Token readStringOrCharToken();
    
    /**
     * Baca komentar dengan kurung kurawal
     * Format: { ini komentar }
     */
    Token readBraceCommentToken();
    
    /**
     * Baca komentar dengan tanda (* *)
     * Format: (* ini komentar *)
     */
    Token readParenStarCommentToken();
    
    /**
     * Baca simbol/operator
     * Format: +, -, *, :=, <>, dll
     */
    Token readSymbolToken();

    // =============================================
    // DFA HELPERS
    // =============================================
    
    /**
     * Transisi DFA dari state saat ini dengan input karakter
     * @param current - state saat ini
     * @param c - karakter input
     * @return state selanjutnya
     */
    State transition(State current, char c) const;
    
    /**
     * Cek apakah state adalah accepting state
     * @param s - state yang dicek
     * @return true jika accepting
     */
    bool isAccepting(State s) const;
    
    /**
     * Build token dari accepting state
     * @param s - accepting state
     * @param lexeme - teks yang sudah dibaca
     * @param startLine - baris awal token
     * @param startCol - kolom awal token
     * @return Token yang sesuai
     */
    Token buildTokenFromAcceptState(State s, const std::string& lexeme, 
                                    int startLine, int startCol) const;
};

#endif
