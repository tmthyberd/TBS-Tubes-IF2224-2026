#include "token.hpp"

/**
 * KERANGKA (SKELETON) - Token Implementation
 * 
 * PETUNJUK IMPLEMENTASI:
 * - Tambahkan case untuk setiap TokenType yang ada di token.hpp
 */

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        // =============================================
        // LITERALS - Sudah diimplementasi sebagai contoh
        // =============================================
        case TokenType::INTCON:  return "intcon";
        case TokenType::REALCON: return "realcon";
        case TokenType::CHARCON: return "charcon";
        case TokenType::STRING:  return "string";

        // =============================================
        // OPERATORS - Contoh parsial
        // =============================================
        case TokenType::NOTSY:   return "notsy";
        case TokenType::PLUS:    return "plus";
        case TokenType::MINUS:   return "minus";
        
        // TODO: Tambahkan case untuk operator lainnya
        // case TokenType::TIMES:   return "times";
        // case TokenType::IDIV:    return "idiv";
        // case TokenType::RDIV:    return "rdiv";
        // case TokenType::IMOD:    return "imod";
        // ... dst

        // =============================================
        // DELIMITERS - Contoh parsial
        // =============================================
        case TokenType::LPARENT: return "lparent";
        case TokenType::RPARENT: return "rparent";
        
        // TODO: Tambahkan case untuk delimiter lainnya
        // case TokenType::LBRACK:    return "lbrack";
        // case TokenType::RBRACK:    return "rbrack";
        // ... dst

        // =============================================
        // KEYWORDS - Contoh parsial
        // =============================================
        case TokenType::CONSTSY: return "constsy";
        case TokenType::VARSY:   return "varsy";
        case TokenType::BEGINSY: return "beginsy";
        case TokenType::ENDSY:   return "endsy";
        case TokenType::IFSY:    return "ifsy";
        
        // TODO: Tambahkan case untuk keyword lainnya
        // case TokenType::TYPESY:      return "typesy";
        // case TokenType::FUNCTIONSY:  return "functionsy";
        // ... dst

        // =============================================
        // GENERIC - Sudah lengkap
        // =============================================
        case TokenType::IDENT:       return "ident";
        case TokenType::COMMENT:     return "comment";
        case TokenType::END_OF_FILE: return "eof";
        case TokenType::LEX_ERROR:   return "lex_error";
        
        default: return "unknown";
    }
}

std::string tokenToOutputString(const Token& token) {
    std::string name = tokenTypeToString(token.type);

    // Token-token yang perlu menampilkan lexeme-nya
    switch (token.type) {
        case TokenType::IDENT:
        case TokenType::INTCON:
        case TokenType::REALCON:
        case TokenType::CHARCON:
        case TokenType::STRING:
        case TokenType::COMMENT:
        case TokenType::LEX_ERROR:
            return name + " (" + token.lexeme + ")";
        default:
            return name;
    }
}
