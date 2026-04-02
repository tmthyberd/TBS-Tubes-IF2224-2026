#include "lexer.h"
#include <cctype>
#include <algorithm>

// Constructor & Helper Functions

Lexer::Lexer(const std::string& sourceCode)
    : source(sourceCode), pos(0), line(1), column(1),
      currentState(State::START), lexeme(""),
      tokenStartLine(1), tokenStartColumn(1) {}

bool Lexer::isAtEnd() const {
    return pos >= source.length();
}

char Lexer::readChar() {
    if (isAtEnd()) {
        return '\0';
    }
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

char Lexer::currentChar() const {
    if (isAtEnd()) {
        return '\0';
    }
    return source[pos];
}

void Lexer::resetState() {
    currentState = State::START;
    lexeme = "";
    tokenStartLine = line;
    tokenStartColumn = column;
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNum(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

std::string Lexer::toLower(const std::string& str) const {
    std::string result = str;
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }
    return result;
}

Token Lexer::makeToken(TokenType type, const std::string& value) {
    return Token(type, value, tokenStartLine, tokenStartColumn);
}

Token Lexer::makeError(const std::string& message) {
    return Token(TokenType::ERROR, message, tokenStartLine, tokenStartColumn);
}

// Keyword Recognition

TokenType Lexer::identifyKeyword(const std::string& ident) const {
    std::string lower = toLower(ident);
    
    // Keyword logika
    if (lower == "not")       return TokenType::NOTSY;
    if (lower == "and")       return TokenType::ANDSY;
    if (lower == "or")        return TokenType::ORSY;
    
    // Operator
    if (lower == "div")       return TokenType::IDIV;
    if (lower == "mod")       return TokenType::IMOD;
    
    // Keyword deklarasi
    if (lower == "const")     return TokenType::CONSTSY;
    if (lower == "type")      return TokenType::TYPESY;
    if (lower == "var")       return TokenType::VARSY;
    if (lower == "function")  return TokenType::FUNCTIONSY;
    if (lower == "procedure") return TokenType::PROCEDURESY;
    if (lower == "array")     return TokenType::ARRAYSY;
    if (lower == "record")    return TokenType::RECORDSY;
    if (lower == "program")   return TokenType::PROGRAMSY;
    
    // Keyword kontrol
    if (lower == "begin")     return TokenType::BEGINSY;
    if (lower == "if")        return TokenType::IFSY;
    if (lower == "case")      return TokenType::CASESY;
    if (lower == "repeat")    return TokenType::REPEATSY;
    if (lower == "while")     return TokenType::WHILESY;
    if (lower == "for")       return TokenType::FORSY;
    if (lower == "end")       return TokenType::ENDSY;
    if (lower == "else")      return TokenType::ELSESY;
    if (lower == "until")     return TokenType::UNTILSY;
    if (lower == "of")        return TokenType::OFSY;
    if (lower == "do")        return TokenType::DOSY;
    if (lower == "to")        return TokenType::TOSY;
    if (lower == "downto")    return TokenType::DOWNTOSY;
    if (lower == "then")      return TokenType::THENSY;
    
    // Bukan keyword, berarti identifier
    return TokenType::IDENT;
}

// Main Tokenization Logic (DFA Implementation)
Token Lexer::getNextToken() {
    resetState();
    
    // Skip whitespace
    while (!isAtEnd() && isWhitespace(currentChar())) {
        readChar();
    }
    
    // Cek EOF
    if (isAtEnd()) {
        return makeToken(TokenType::END_OF_FILE);
    }
    
    // Catat posisi awal token
    tokenStartLine = line;
    tokenStartColumn = column;
    
    // Baca karakter pertama dan mulai transisi DFA
    char c = readChar();
    lexeme = "";
    lexeme += c;
    
    // Single-character tokens (langsung ke final state)
    
    if (c == '+') return makeToken(TokenType::PLUS);
    if (c == '-') return makeToken(TokenType::MINUS);
    if (c == '*') return makeToken(TokenType::TIMES);
    if (c == '/') return makeToken(TokenType::RDIV);
    if (c == ')') return makeToken(TokenType::RPARENT);
    if (c == '[') return makeToken(TokenType::LBRACK);
    if (c == ']') return makeToken(TokenType::RBRACK);
    if (c == ',') return makeToken(TokenType::COMMA);
    if (c == ';') return makeToken(TokenType::SEMICOLON);
    if (c == '.') return makeToken(TokenType::PERIOD);
    
    // Colon : atau Becomes :=
    if (c == ':') {
        currentState = State::SAW_COLON;
        if (!isAtEnd() && currentChar() == '=') {
            readChar();
            return makeToken(TokenType::BECOMES);
        }
        return makeToken(TokenType::COLON);
    }
    
    // Less < atau Leq <= atau Neq <>
    if (c == '<') {
        currentState = State::SAW_LESS;
        if (!isAtEnd()) {
            char next = currentChar();
            if (next == '=') {
                readChar();
                return makeToken(TokenType::LEQ);
            }
            if (next == '>') {
                readChar();
                return makeToken(TokenType::NEQ);
            }
        }
        return makeToken(TokenType::LSS);
    }
    
    // Greater > atau Geq >=
    if (c == '>') {
        currentState = State::SAW_GREATER;
        if (!isAtEnd() && currentChar() == '=') {
            readChar();
            return makeToken(TokenType::GEQ);
        }
        return makeToken(TokenType::GTR);
    }
    
    // Equal == (= tunggal adalah unknown)
    if (c == '=') {
        currentState = State::SAW_EQUAL;
        if (!isAtEnd() && currentChar() == '=') {
            readChar();
            return makeToken(TokenType::EQL);
        }

        return makeToken(TokenType::UNKNOWN, "=");
    }
    
    // Left Parenthesis ( atau Comment (*
    
    if (c == '(') {
        currentState = State::SAW_LPAREN;
        if (!isAtEnd() && currentChar() == '*') {
            readChar(); // consume '*'
            currentState = State::IN_COMMENT_PAREN;
            lexeme = "";
            
            // Baca sampai menemukan *)
            while (!isAtEnd()) {
                c = readChar();
                if (c == '*') {
                    currentState = State::IN_COMMENT_PAREN_STAR;
                    if (!isAtEnd() && currentChar() == ')') {
                        readChar(); // consume ')'
                        // Kembalikan comment dengan format (* ... *)
                        return makeToken(TokenType::COMMENT, "(*" + lexeme + "*)");
                    }
                    lexeme += c;
                } else {
                    lexeme += c;
                }
            }
            // EOF tanpa penutup - error
            return makeError("Unclosed comment starting with (*");
        }
        return makeToken(TokenType::LPARENT);
    }
    
    if (c == '{') {
        currentState = State::IN_COMMENT_BRACE;
        lexeme = "";
        
        // Baca sampai menemukan }
        while (!isAtEnd()) {
            c = readChar();
            if (c == '}') {
                return makeToken(TokenType::COMMENT, "{" + lexeme + "}");
            }
            lexeme += c;
        }
        // EOF tanpa penutup - error
        return makeError("Unclosed comment starting with {");
    }
    
    // } tanpa { - unknown
    if (c == '}') {
        return makeToken(TokenType::UNKNOWN, "}");
    }
    
    // String dan Charcon '...'
    
    if (c == '\'') {
        currentState = State::IN_STRING;
        lexeme = "";
        
        while (!isAtEnd()) {
            c = readChar();
            
            if (c == '\'') {
                // Cek apakah ini escape '' atau penutup
                currentState = State::IN_STRING_ESCAPE;
                if (!isAtEnd() && currentChar() == '\'') {
                    // Escape: '' -> tambahkan satu ' ke lexeme
                    readChar();
                    lexeme += '\'';
                    currentState = State::IN_STRING;
                } else {
                    // Penutup string/charcon
                    // Tentukan apakah charcon atau string
                    if (lexeme.length() == 1) {
                        return makeToken(TokenType::CHARCON, "'" + lexeme + "'");
                    } else {
                        // String (termasuk string kosong)
                        return makeToken(TokenType::STRING, "'" + lexeme + "'");
                    }
                }
            } else if (c == '\n' || c == '\r') {
                // String tidak boleh multiline
                return makeError("String cannot span multiple lines");
            } else {
                lexeme += c;
            }
        }
        // EOF tanpa penutup - error
        return makeError("Unclosed string literal");
    }
    
    // Integer dan Real number
    
    if (isDigit(c)) {
        currentState = State::IN_INTEGER;
        
        // Baca semua digit
        while (!isAtEnd() && isDigit(currentChar())) {
            lexeme += readChar();
        }
        
        // Cek apakah ada titik desimal
        if (!isAtEnd() && currentChar() == '.') {
            size_t dotPos = pos;
            int dotLine = line;
            int dotColumn = column;
            
            char dot = readChar();
            
            if (!isAtEnd() && isDigit(currentChar())) {
                currentState = State::IN_REAL;
                lexeme += dot;
                
                while (!isAtEnd() && isDigit(currentChar())) {
                    lexeme += readChar();
                }
                return makeToken(TokenType::REALCON, lexeme);
            } else {
                pos = dotPos;
                line = dotLine;
                column = dotColumn;
                
                return makeToken(TokenType::INTCON, lexeme);
            }
        }
        
        return makeToken(TokenType::INTCON, lexeme);
    }
    
    // Identifier dan Keyword
    
    if (isAlpha(c)) {
        currentState = State::IN_IDENT;
        
        // Baca semua huruf dan angka
        while (!isAtEnd() && isAlphaNum(currentChar())) {
            lexeme += readChar();
        }
        
        // Cek apakah keyword atau identifier
        TokenType type = identifyKeyword(lexeme);
        
        if (type == TokenType::IDENT) {
            return makeToken(TokenType::IDENT, lexeme);
        } else {
            return makeToken(type);
        }
    }
    
    // Unknown character
    
    return makeToken(TokenType::UNKNOWN, lexeme);
}


std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        Token token = getNextToken();
        
        // Tambahkan token
        if (token.type != TokenType::END_OF_FILE) {
            tokens.push_back(token);
        } else {
            break;
        }
    }
    
    // Cek apakah masih ada karakter tersisa
    if (!isAtEnd()) {
        Token token = getNextToken();
        if (token.type != TokenType::END_OF_FILE) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}
