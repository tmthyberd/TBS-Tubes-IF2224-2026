#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token.h"


enum class State {
    // State awal
    START,
    
    // State untuk angka (intcon/realcon)
    IN_INTEGER,    
    IN_REAL_DOT,   
    IN_REAL,       
    
    // State untuk identifier/keyword
    IN_IDENT,      
    
    // State untuk string/charcon
    IN_STRING,     
    IN_STRING_ESCAPE, 
    
    // State untuk komentar kurung kurawal { }
    IN_COMMENT_BRACE,   // Di dalam komentar { ... }
    
    // State untuk komentar (* *)
    SAW_LPAREN,    
    IN_COMMENT_PAREN,  
    IN_COMMENT_PAREN_STAR, 
    
    // State untuk operator multi-karakter
    SAW_COLON,     
    SAW_LESS,      
    SAW_GREATER,   
    SAW_EQUAL,     
    
    // State final 
    DONE           
};

class Lexer {
public:
    explicit Lexer(const std::string& sourceCode);
    
    std::vector<Token> tokenize();
    Token getNextToken();
    bool isAtEnd() const;

private:
    std::string source;
    size_t pos;        
    int line;          
    int column;        
    State currentState;
    std::string lexeme;
    int tokenStartLine;
    int tokenStartColumn;  

    char readChar();
    char currentChar() const;
    void resetState();

    TokenType identifyKeyword(const std::string& ident) const;
    std::string toLower(const std::string& str) const;
    

    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNum(char c) const;
    bool isWhitespace(char c) const;

    Token makeToken(TokenType type, const std::string& value = "");
    Token makeError(const std::string& message);
};

#endif
