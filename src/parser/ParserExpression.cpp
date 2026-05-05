#include "Parser.hpp"
#include "core/ErrorHandler.hpp"


// Helper
bool isRelationalOp(TokenType type) {
    return type == TokenType::EQL || type == TokenType::NEQ ||
           type == TokenType::GTR || type == TokenType::GEQ ||
           type == TokenType::LSS || type == TokenType::LEQ;
}

bool isAdditiveOp(TokenType type) {
    return type == TokenType::PLUS || type == TokenType::MINUS || type == TokenType::ORSY;
}

bool isMultiplicativeOp(TokenType type) {
    return type == TokenType::TIMES || type == TokenType::IDIV ||
           type == TokenType::RDIV || type == TokenType::IMOD || type == TokenType::ANDSY;
}


// Operator Node
std::unique_ptr<ParseTreeNode> Parser::parse_relational_operator() {
    auto node = std::make_unique<ParseTreeNode>("<relational-operator>");
    node->addToken(ts.advance());  
    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_additive_operator() {
    auto node = std::make_unique<ParseTreeNode>("<additive-operator>");
    node->addToken(ts.advance());
    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_multiplicative_operator() {
    auto node = std::make_unique<ParseTreeNode>("<multiplicative-operator>");
    node->addToken(ts.advance());
    return node;
}


// Index List & Variable
std::unique_ptr<ParseTreeNode> Parser::parse_index_list() {
    auto node = std::make_unique<ParseTreeNode>("<index-list>");

    if (ts.check(TokenType::INTCON) || ts.check(TokenType::CHARCON) || ts.check(TokenType::IDENT)) {
        node->addToken(ts.advance());
    } else {
        ErrorHandler::unexpectedToken(ts.peek(), "intcon, charcon, or ident", "parse_index_list");
    }

    while (ts.check(TokenType::COMMA)) {
        node->addToken(ts.advance());           
        node->addChild(parse_index_list());   
    }

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_component_variable() {
    // logic component-variable udah dihandle di parse_variable() pake loop, jadi panggil parse_variable() aja
    return nullptr;
}

std::unique_ptr<ParseTreeNode> Parser::parse_variable() {
    auto node = std::make_unique<ParseTreeNode>("<variable>");

    node->addToken(ts.expect(TokenType::IDENT, "parse_variable"));

    while (ts.check(TokenType::LBRACK) || ts.check(TokenType::PERIOD)) {
        auto compNode = std::make_unique<ParseTreeNode>("<component-variable>");

        compNode->addChild(std::move(node));

        if (ts.check(TokenType::LBRACK)) {
            compNode->addToken(ts.advance());          
            compNode->addChild(parse_index_list());     
            compNode->addToken(ts.expect(TokenType::RBRACK, "parse_variable")); 
        } else {
            compNode->addToken(ts.advance());         
            compNode->addToken(ts.expect(TokenType::IDENT, "parse_variable"));
        }

        node = std::make_unique<ParseTreeNode>("<variable>");
        node->addChild(std::move(compNode));
    }

    return node;
}


// Parameter List
std::unique_ptr<ParseTreeNode> Parser::parse_parameter_list() {
    auto node = std::make_unique<ParseTreeNode>("<parameter-list>");

    node->addChild(parse_expression());  

    while (ts.check(TokenType::COMMA)) {
        node->addToken(ts.advance());       
        node->addChild(parse_expression()); 
    }

    return node;
}


// Factor
std::unique_ptr<ParseTreeNode> Parser::parse_factor() {
    auto node = std::make_unique<ParseTreeNode>("<factor>");

    // Kasus 1: literal langsung
    if (ts.check(TokenType::INTCON) || ts.check(TokenType::REALCON) ||
        ts.check(TokenType::CHARCON) || ts.check(TokenType::STRING)) {
        node->addToken(ts.advance());

    // Kasus 2: not factor
    } else if (ts.check(TokenType::NOTSY)) {
        node->addToken(ts.advance());      
        node->addChild(parse_factor());     

    // Kasus 3: ( expression )
    } else if (ts.check(TokenType::LPARENT)) {
        node->addToken(ts.advance());         
        node->addChild(parse_expression());                      
        node->addToken(ts.expect(TokenType::RPARENT, "parse_factor")); 

    // Kasus 4: ident
    } else if (ts.check(TokenType::IDENT)) {
        TokenType next = ts.peek(1).type;

        if (next == TokenType::LPARENT) {
            node->addChild(parse_procedure_function_call());

        } else if (next == TokenType::LBRACK || next == TokenType::PERIOD) {
            node->addChild(parse_variable());

        } else {
            node->addChild(parse_procedure_function_call());
        }

    } else {
        ErrorHandler::unexpectedToken(ts.peek(), "factor", "parse_factor");
    }

    return node;
}


// Term
std::unique_ptr<ParseTreeNode> Parser::parse_term() {
    auto node = std::make_unique<ParseTreeNode>("<term>");

    node->addChild(parse_factor()); 

    while (isMultiplicativeOp(ts.peek().type)) {
        node->addChild(parse_multiplicative_operator());
        node->addChild(parse_factor());
    }

    return node;
}


// Simple Expression
std::unique_ptr<ParseTreeNode> Parser::parse_simple_expression() {
    auto node = std::make_unique<ParseTreeNode>("<simple-expression>");

    if (ts.check(TokenType::PLUS) || ts.check(TokenType::MINUS)) {
        node->addToken(ts.advance());
    }

    node->addChild(parse_term());   

    while (isAdditiveOp(ts.peek().type)) {
        node->addChild(parse_additive_operator());
        node->addChild(parse_term());
    }

    return node;
}


// Expression
std::unique_ptr<ParseTreeNode> Parser::parse_expression() {
    auto node = std::make_unique<ParseTreeNode>("<expression>");

    node->addChild(parse_simple_expression());

    if (isRelationalOp(ts.peek().type)) {
        node->addChild(parse_relational_operator());
        node->addChild(parse_simple_expression());
    }

    return node;
}
