#include "lexer.hpp"
#include <cctype>

/**
 * KERANGKA (SKELETON) - Lexer Implementation
 * 
 * PETUNJUK IMPLEMENTASI:
 * - Implementasi penuh ada di file lexer.cpp asli
 * - File ini berisi contoh parsial untuk dipelajari
 */

// =============================================
// CONSTRUCTOR
// =============================================

Lexer::Lexer(const std::string& input)
    : source(input), pos(0), line(1), column(1) {}

// =============================================
// TOKENIZE - Sudah lengkap
// =============================================

std::vector<Token> Lexer::tokenize(bool includeComments) {
    std::vector<Token> tokens;

    while (true) {
        Token tok = nextToken();

        // Skip komentar jika tidak diminta
        if (tok.type == TokenType::COMMENT && !includeComments) {
            continue;
        }

        tokens.push_back(tok);

        if (tok.type == TokenType::END_OF_FILE) {
            break;
        }
    }

    return tokens;
}

// =============================================
// NEXT TOKEN - Sudah lengkap
// =============================================

Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", line, column);
    }

    char c = peek();

    // Huruf -> keyword atau identifier
    if (isLetter(c)) {
        return readWordLikeToken();
    }

    // Digit -> angka (integer atau real)
    if (isDigit(c)) {
        return readNumberToken();
    }

    // Quote -> string atau character
    if (c == '\'') {
        return readStringOrCharToken();
    }

    // Kurung kurawal -> komentar
    if (c == '{') {
        return readBraceCommentToken();
    }

    // (* -> komentar
    if (c == '(' && peekNext() == '*') {
        return readParenStarCommentToken();
    }

    // Simbol/operator lainnya
    return readSymbolToken();
}

// =============================================
// BASIC CURSOR UTILITIES - Sudah lengkap
// =============================================

bool Lexer::isAtEnd() const {
    return pos >= source.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

char Lexer::peekNext() const {
    if (pos + 1 >= source.size()) return '\0';
    return source[pos + 1];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';

    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::retreatOne() {
    if (pos == 0) return;

    pos--;
    char c = source[pos];

    if (c == '\n') {
        line--;
        column = 1;
        // Hitung ulang kolom
        for (size_t i = pos; i > 0 && source[i - 1] != '\n'; --i) {
            column++;
        }
    } else {
        column--;
    }
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[pos] != expected) return false;
    advance();
    return true;
}

// =============================================
// CHARACTER CLASSIFICATION - Sudah lengkap
// =============================================

bool Lexer::isLetter(char c) const {
    return std::isalpha(static_cast<unsigned char>(c));
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::isAlnum(char c) const {
    return std::isalnum(static_cast<unsigned char>(c));
}

char Lexer::toLowerChar(char c) const {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

// =============================================
// WHITESPACE HANDLING - Sudah lengkap
// =============================================

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

// =============================================
// DFA TRANSITION - Contoh Parsial
// =============================================

Lexer::State Lexer::transition(State current, char raw) const {
    char c = toLowerChar(raw);  // Case-insensitive

    switch (current) {
        // State awal: tentukan jalur berdasarkan huruf pertama
        case State::START:
            switch (c) {
                case 'a': return State::A;
                case 'b': return State::B;
                case 'c': return State::C;
                // TODO: Tambahkan huruf lainnya
                // case 'd': return State::D;
                // case 'e': return State::E;
                // case 'f': return State::F;
                // case 'i': return State::I;
                // case 'm': return State::M;
                // case 'n': return State::N;
                // case 'o': return State::O;
                // case 'p': return State::P;
                // case 'r': return State::R;
                // case 't': return State::T;
                // case 'u': return State::U;
                // case 'v': return State::V;
                // case 'w': return State::W;
                default:
                    if (isAlnum(c)) return State::IDENT;
                    return State::ERROR_STATE;
            }

        // =============================================
        // PATH: A -> array, and
        // =============================================
        case State::A:
            if (c == 'r') return State::AR;
            if (c == 'n') return State::AN;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::AR:
            if (c == 'r') return State::ARR;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::ARR:
            if (c == 'a') return State::ARRA;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::ARRA:
            if (c == 'y') return State::ARRAY_FINAL;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::ARRAY_FINAL:
            if (isAlnum(c)) return State::IDENT;  // arrayx -> identifier
            return State::ERROR_STATE;

        case State::AN:
            if (c == 'd') return State::AND_FINAL;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::AND_FINAL:
            if (isAlnum(c)) return State::IDENT;  // andx -> identifier
            return State::ERROR_STATE;

        // =============================================
        // PATH: B -> begin
        // =============================================
        case State::B:
            if (c == 'e') return State::BE;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::BE:
            if (c == 'g') return State::BEG;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::BEG:
            if (c == 'i') return State::BEGI;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::BEGI:
            if (c == 'n') return State::BEGIN_FINAL;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::BEGIN_FINAL:
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;

        // =============================================
        // PATH: C -> case, const
        // =============================================
        case State::C:
            if (c == 'a') return State::CA;
            if (c == 'o') return State::CO;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;

        case State::CA:
            if (c == 's') return State::CAS;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::CAS:
            if (c == 'e') return State::CASE_FINAL;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::CASE_FINAL:
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;

        case State::CO:
            if (c == 'n') return State::CON;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::CON:
            if (c == 's') return State::CONS;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::CONS:
            if (c == 't') return State::CONST_FINAL;
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;
        case State::CONST_FINAL:
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;

        // TODO: Implementasi path keyword lainnya...
        // Lihat file lexer.cpp asli untuk referensi lengkap

        // =============================================
        // IDENTIFIER STATE - selalu terima alphanumeric
        // =============================================
        case State::IDENT:
            if (isAlnum(c)) return State::IDENT;
            return State::ERROR_STATE;

        default:
            return State::ERROR_STATE;
    }
}

// =============================================
// IS ACCEPTING - Contoh Parsial
// =============================================

bool Lexer::isAccepting(State s) const {
    switch (s) {
        // Semua FINAL state dan IDENT adalah accepting
        case State::ARRAY_FINAL:
        case State::AND_FINAL:
        case State::BEGIN_FINAL:
        case State::CASE_FINAL:
        case State::CONST_FINAL:
        // TODO: Tambahkan FINAL state lainnya
        case State::IDENT:
            return true;
        default:
            return false;
    }
}

// =============================================
// BUILD TOKEN FROM ACCEPT STATE - Contoh Parsial
// =============================================

Token Lexer::buildTokenFromAcceptState(State s, const std::string& lexeme, 
                                       int startLine, int startCol) const {
    switch (s) {
        case State::ARRAY_FINAL:
            return Token(TokenType::ARRAYSY, lexeme, startLine, startCol);
        case State::AND_FINAL:
            return Token(TokenType::ANDSY, lexeme, startLine, startCol);
        case State::BEGIN_FINAL:
            return Token(TokenType::BEGINSY, lexeme, startLine, startCol);
        case State::CASE_FINAL:
            return Token(TokenType::CASESY, lexeme, startLine, startCol);
        case State::CONST_FINAL:
            return Token(TokenType::CONSTSY, lexeme, startLine, startCol);
        // TODO: Tambahkan FINAL state lainnya
        
        case State::IDENT:
        default:
            return Token(TokenType::IDENT, lexeme, startLine, startCol);
    }
}

// =============================================
// READ WORD-LIKE TOKEN - Contoh lengkap
// =============================================

Token Lexer::readWordLikeToken() {
    int startLine = line;
    int startCol = column;
    std::string lexeme;
    
    State current = State::START;
    State lastAccept = State::ERROR_STATE;
    size_t lastAcceptPos = pos;
    int lastAcceptLine = line;
    int lastAcceptCol = column;
    std::string lastAcceptLexeme;

    while (!isAtEnd()) {
        char c = peek();
        State next = transition(current, c);

        if (next == State::ERROR_STATE) {
            break;
        }

        advance();
        lexeme += c;
        current = next;

        // Simpan jika accepting
        if (isAccepting(current)) {
            lastAccept = current;
            lastAcceptPos = pos;
            lastAcceptLine = line;
            lastAcceptCol = column;
            lastAcceptLexeme = lexeme;
        }
    }

    // Gunakan last accept state
    if (lastAccept != State::ERROR_STATE) {
        // Restore posisi ke setelah token yang diterima
        while (pos > lastAcceptPos) {
            retreatOne();
        }
        return buildTokenFromAcceptState(lastAccept, lastAcceptLexeme, 
                                         startLine, startCol);
    }

    // Tidak ada yang cocok -> error
    return Token(TokenType::LEX_ERROR, lexeme, startLine, startCol);
}

// =============================================
// READ NUMBER TOKEN - TODO: Implementasi
// =============================================

Token Lexer::readNumberToken() {
    int startLine = line;
    int startCol = column;
    std::string lexeme;

    // TODO: Implementasi membaca angka
    // 1. Baca digit pertama
    // 2. Baca digit berikutnya
    // 3. Cek apakah ada titik desimal -> REALCON
    // 4. Cek apakah ada exponent (e/E) -> REALCON
    // 5. Jika tidak ada titik/exponent -> INTCON

    // Contoh sederhana (hanya integer):
    while (!isAtEnd() && isDigit(peek())) {
        lexeme += advance();
    }

    return Token(TokenType::INTCON, lexeme, startLine, startCol);
}

// =============================================
// READ STRING/CHAR TOKEN - TODO: Implementasi
// =============================================

Token Lexer::readStringOrCharToken() {
    int startLine = line;
    int startCol = column;
    std::string lexeme;

    // TODO: Implementasi membaca string/char
    // 1. Consume opening quote
    // 2. Baca karakter sampai closing quote
    // 3. Handle escape sequence ''
    // 4. Tentukan CHARCON (1 char) atau STRING (>1 char)

    // Placeholder:
    advance();  // skip opening '
    
    while (!isAtEnd() && peek() != '\'') {
        lexeme += advance();
    }
    
    if (!isAtEnd()) {
        advance();  // skip closing '
    }

    // Sementara return STRING
    return Token(TokenType::STRING, lexeme, startLine, startCol);
}

// =============================================
// READ BRACE COMMENT - TODO: Implementasi
// =============================================

Token Lexer::readBraceCommentToken() {
    int startLine = line;
    int startCol = column;
    std::string lexeme;

    // TODO: Implementasi membaca komentar { ... }
    // 1. Consume opening {
    // 2. Baca semua sampai }
    // 3. Return COMMENT token

    advance();  // skip {
    
    while (!isAtEnd() && peek() != '}') {
        lexeme += advance();
    }
    
    if (!isAtEnd()) {
        advance();  // skip }
    }

    return Token(TokenType::COMMENT, lexeme, startLine, startCol);
}

// =============================================
// READ PAREN-STAR COMMENT - TODO: Implementasi
// =============================================

Token Lexer::readParenStarCommentToken() {
    int startLine = line;
    int startCol = column;
    std::string lexeme;

    // TODO: Implementasi membaca komentar (* ... *)
    // 1. Consume (*
    // 2. Baca sampai *)
    // 3. Return COMMENT token

    advance();  // skip (
    advance();  // skip *
    
    while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == ')') {
            advance();  // skip *
            advance();  // skip )
            break;
        }
        lexeme += advance();
    }

    return Token(TokenType::COMMENT, lexeme, startLine, startCol);
}

// =============================================
// READ SYMBOL TOKEN - TODO: Implementasi lengkap
// =============================================

Token Lexer::readSymbolToken() {
    int startLine = line;
    int startCol = column;
    char c = advance();

    switch (c) {
        // =============================================
        // SINGLE CHARACTER SYMBOLS - Contoh
        // =============================================
        case '+': return Token(TokenType::PLUS, "+", startLine, startCol);
        case '-': return Token(TokenType::MINUS, "-", startLine, startCol);
        // TODO: case '*': return Token(TokenType::TIMES, "*", startLine, startCol);
        // TODO: case '/': return Token(TokenType::RDIV, "/", startLine, startCol);
        // TODO: case '=': return Token(TokenType::EQL, "=", startLine, startCol);
        
        case '(': return Token(TokenType::LPARENT, "(", startLine, startCol);
        case ')': return Token(TokenType::RPARENT, ")", startLine, startCol);
        // TODO: case '[': return Token(TokenType::LBRACK, "[", startLine, startCol);
        // TODO: case ']': return Token(TokenType::RBRACK, "]", startLine, startCol);
        // TODO: case ',': return Token(TokenType::COMMA, ",", startLine, startCol);
        // TODO: case ';': return Token(TokenType::SEMICOLON, ";", startLine, startCol);
        // TODO: case '.': return Token(TokenType::PERIOD, ".", startLine, startCol);

        // =============================================
        // MULTI-CHARACTER SYMBOLS - TODO: Implementasi
        // =============================================
        case ':':
            // Cek apakah := (becomes) atau : (colon)
            if (match('=')) {
                return Token(TokenType::BECOMES, ":=", startLine, startCol);
            }
            // TODO: return Token(TokenType::COLON, ":", startLine, startCol);
            break;

        case '<':
            // Cek apakah <> (neq), <= (leq), atau < (lss)
            // TODO: Implementasi
            break;

        case '>':
            // Cek apakah >= (geq) atau > (gtr)
            // TODO: Implementasi
            break;

        default:
            break;
    }

    // Unknown symbol -> error
    return Token(TokenType::LEX_ERROR, std::string(1, c), startLine, startCol);
}
