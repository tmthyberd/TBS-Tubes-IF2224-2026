#include "Parser.hpp"

std::unique_ptr<ParseTreeNode> Parser::parse_program() {
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<program>"));

    node->addChild(parse_program_header());

    node->addChild(parse_declaration_part());

    node->addChild(parse_compound_statement());

    const Token& periodToken = ts.expect(TokenType::PERIOD, "<program>");
    node->addToken(periodToken);

    return node;
}

// <program-header> ::= 'program' <ident>
std::unique_ptr<ParseTreeNode> Parser::parse_program_header()
{
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<program-header>"));

    const Token& programToken = ts.expect(TokenType::PROGRAMSY, "<program-header>");
    node->addToken(programToken);

    const Token& identToken = ts.expect(TokenType::IDENT, "<program-header>");
    node->addToken(identToken);

    const Token& semicolonToken = ts.expect(TokenType::SEMICOLON, "<program-header>");
    node->addToken(semicolonToken);

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_declaration_part(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<declaration-part>"));

    while (ts.check(TokenType::CONSTSY)){
        node->addChild(parse_const_declaration());
    }

    while (ts.check(TokenType::TYPESY)){
        node->addChild(parse_type_declaration());
    }

    while (ts.check(TokenType::VARSY)){
        node->addChild(parse_var_declaration());
    }

    while (ts.check(TokenType::PROCEDURESY) || ts.check(TokenType::FUNCTIONSY)){
        node->addChild(parse_subprogram_declaration());
    }

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_const_declaration(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<const-declaration>"));

    const Token& constToken = ts.expect(TokenType::CONSTSY, "<const-declaration>");
    node->addToken(constToken);

    do {
        const Token& identToken = ts.expect(TokenType::IDENT, "<const-declaration>");
        node->addToken(identToken);

        const Token& eqlToken = ts.expect(TokenType::EQL, "<const-declaration>");
        node->addToken(eqlToken);

        node->addChild(parse_constant());

        const Token& semicolonToken = ts.expect(TokenType::SEMICOLON, "<const-declaration>");
        node->addToken(semicolonToken);
    } while (ts.check(TokenType::IDENT));
    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_constant(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<constant>"));

    if (ts.check(TokenType::INTCON)){
        const Token& intToken = ts.expect(TokenType::INTCON, "<constant>");
        node->addToken(intToken);
    }
    else if (ts.check(TokenType::CHARCON)){
        const Token& charToken = ts.expect(TokenType::CHARCON, "<constant>");
        node->addToken(charToken);
    }
    else if (ts.check(TokenType::STRING)){
        const Token& strToken = ts.expect(TokenType::STRING, "<constant>");
        node->addToken(strToken);
    }
    else if (ts.check(TokenType::REALCON)){
        const Token& realToken = ts.expect(TokenType::REALCON, "<constant>");
        node->addToken(realToken);
    }
    else if (ts.check(TokenType::IDENT)){
        const Token& identToken = ts.expect(TokenType::IDENT, "<constant>");
        node->addToken(identToken);
    }
    else if (ts.check(TokenType::PLUS) || ts.check(TokenType::MINUS)){
        if (ts.check(TokenType::PLUS)){
            const Token& plusToken = ts.expect(TokenType::PLUS, "<constant>");
            node->addToken(plusToken);
        }
        else {
            const Token& minusToken = ts.expect(TokenType::MINUS, "<constant>");
            node->addToken(minusToken);
        }

        if (ts.check(TokenType::INTCON)){
            const Token& intToken = ts.expect(TokenType::INTCON, "<constant>");
            node->addToken(intToken);
        }
        else if (ts.check(TokenType::REALCON)){
            const Token& realToken = ts.expect(TokenType::REALCON, "<constant>");
            node->addToken(realToken);
        }
        else if (ts.check(TokenType::IDENT)){
            const Token& identToken = ts.expect(TokenType::IDENT, "<constant>");
            node->addToken(identToken);
        }
    }
    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_type_declaration(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<type-declaration>"));

    const Token& typesyToken = ts.expect(TokenType::TYPESY, "<type-declaration>");
    node->addToken(typesyToken);

    do {
        const Token& identToken = ts.expect(TokenType::IDENT, "<type-declaration>");
        node->addToken(identToken);

        const Token& eqToken = ts.expect(TokenType::EQL, "<type-declaration>");
        node->addToken(eqToken);

        node->addChild(parse_type());

        const Token& semicolonToken = ts.expect(TokenType::SEMICOLON, "<type-declaration>");
        node->addToken(semicolonToken);
    } while (ts.check(TokenType::IDENT));
    
    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_var_declaration(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<var-declaration>"));

    const Token& varsyToken = ts.expect(TokenType::VARSY, "<var-declaration>");
    node->addToken(varsyToken);

    do {
        node->addChild(parse_identifier_list());

        const Token& colonToken = ts.expect(TokenType::COLON, "<var-declaration>");
        node->addToken(colonToken);

        node->addChild(parse_type());

        const Token& semicolonToken = ts.expect(TokenType::SEMICOLON, "<var-declaration>");
        node->addToken(semicolonToken);
    } while(ts.check(TokenType::IDENT));

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_identifier_list(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<identifier-list>"));

    const Token& identToken = ts.expect(TokenType::IDENT, "<identifier-list>");
    node->addToken(identToken);

    while(ts.check(TokenType::COMMA)){
        const Token& commaToken = ts.expect(TokenType::COMMA, "<identifier-list>");
        node->addToken(commaToken);

        const Token& identToken = ts.expect(TokenType::IDENT, "<identifier-list>");
        node->addToken(identToken);
    }

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_type(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<type>"));

    if (ts.check(TokenType::ARRAYSY)){
        node->addChild(parse_array_type());
    }
    else if (ts.check(TokenType::LPARENT)){
        node->addChild(parse_enumerated());
    }
    else if (ts.check(TokenType::RECORDSY)){
        node->addChild(parse_record_type());
    }
    else if (ts.check(TokenType::IDENT) && ts.peek(1).type == TokenType::PERIOD && ts.peek(2).type == TokenType::PERIOD){
        node->addChild(parse_range());
    }
    else if (ts.check(TokenType::IDENT)){
        const Token& identToken = ts.expect(TokenType::IDENT, "<type>");
        node->addToken(identToken);
    }
    else if (ts.check(TokenType::INTCON) || ts.check(TokenType::REALCON) ||
             ts.check(TokenType::CHARCON) || ts.check(TokenType::PLUS) ||
             ts.check(TokenType::MINUS)){
        node->addChild(parse_range());
    }
    else {
        node->addChild(parse_range());
    }   

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_array_type(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<array-type>"));

    const Token& arrayToken = ts.expect(TokenType::ARRAYSY, "<array-type");
    node->addToken(arrayToken);

    const Token& lbrackToken = ts.expect(TokenType::LBRACK, "<array-type");
    node->addToken(lbrackToken);

    if (ts.check(TokenType::IDENT) && (ts.peek(1).type != TokenType::PERIOD)){
        const Token& identToken = ts.expect(TokenType::IDENT, "<array-type>");
        node->addToken(identToken);
    }
    else {
        node->addChild(parse_range());
    }

    const Token& rbrackToken = ts.expect(TokenType::RBRACK, "<array-type>");
    node->addToken(rbrackToken);

    const Token& ofsyToken = ts.expect(TokenType::OFSY, "<array-type>");
    node->addToken(ofsyToken);    

    node->addChild(parse_type());

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_range(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<range>"));

    node->addChild(parse_constant());

    const Token& period1 = ts.expect(TokenType::PERIOD, "<range>");
    node->addToken(period1);

    const Token& period2 = ts.expect(TokenType::PERIOD, "<range>");
    node->addToken(period2);

    node->addChild(parse_constant());

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_enumerated(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<enumerated>"));

    const Token& lparentToken = ts.expect(TokenType::LPARENT, "<enumerated>");
    node->addToken(lparentToken);

    const Token& identToken = ts.expect(TokenType::IDENT, "<enumerated>");
    node->addToken(identToken);

    while (ts.check(TokenType::COMMA)){
        const Token& commaToken = ts.expect(TokenType::COMMA, "<enumerated>");
        node->addToken(commaToken);

        const Token& identToken = ts.expect(TokenType::IDENT, "<enumerated>");
        node->addToken(identToken);
    }

    const Token& rparentToken = ts.expect(TokenType::RPARENT, "<enumerated>");
    node->addToken(rparentToken);

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_record_type(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<record-type>"));

    const Token& recordToken = ts.expect(TokenType::RECORDSY, "<record-type>");
    node->addToken(recordToken);

    node->addChild(parse_field_list());

    const Token& endToken = ts.expect(TokenType::ENDSY, "<record-type>");
    node->addToken(endToken);

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_field_list(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<field-list>"));

    node->addChild(parse_field_part());

    while(ts.check(TokenType::SEMICOLON) && ts.peek(1).type == TokenType::IDENT){
        const Token& semicolonToken = ts.expect(TokenType::SEMICOLON, "<field-list>");
        node->addToken(semicolonToken);

        node->addChild(parse_field_part());
    }
    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_field_part(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<field-part>"));

    node->addChild(parse_identifier_list());

    const Token& colonToken = ts.expect(TokenType::COLON, "<field-part>");
    node->addToken(colonToken);

    node->addChild(parse_type());

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_subprogram_declaration(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<subprogram-declaration>"));

    if (ts.check(TokenType::PROCEDURESY)){
        node->addChild(parse_procedure_declaration());
    }
    else if (ts.check(TokenType::FUNCTIONSY)){
        node->addChild(parse_function_declaration());
    }

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_procedure_declaration(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<procedure-declaration>"));

    const Token& procedureToken = ts.expect(TokenType::PROCEDURESY, "<procedure-declaration>");
    node->addToken(procedureToken);

    const Token& identToken = ts.expect(TokenType::IDENT, "<procedure-declaration>");
    node->addToken(identToken);

    if (ts.check(TokenType::LPARENT)){
        node->addChild(parse_formal_parameter_list());
    }

    const Token& semicolon1Token = ts.expect(TokenType::SEMICOLON, "<procedure-declaration>");
    node->addToken(semicolon1Token);

    node->addChild(parse_block());

    const Token& semicolon2Token = ts.expect(TokenType::SEMICOLON, "<procedure-declaration>");
    node->addToken(semicolon2Token);

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_function_declaration(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<function-declaration>"));

    const Token& functionToken = ts.expect(TokenType::FUNCTIONSY, "<function-declaration>");
    node->addToken(functionToken);

    const Token& ident1Token = ts.expect(TokenType::IDENT, "<function-declaration>");
    node->addToken(ident1Token);


    if (ts.check(TokenType::LPARENT)){
        node->addChild(parse_formal_parameter_list());
    }

    const Token& colonToken = ts.expect(TokenType::COLON, "<function-declaration>");
    node->addToken(colonToken);

    const Token& ident2Token = ts.expect(TokenType::IDENT, "<function-declaration>");
    node->addToken(ident2Token);

    const Token& semicolon1Token = ts.expect(TokenType::SEMICOLON, "<function-declaration>");
    node->addToken(semicolon1Token);

    node->addChild(parse_block());

    const Token& semicolon2Token = ts.expect(TokenType::SEMICOLON, "<function-declaration>");
    node->addToken(semicolon2Token);

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_block(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<block>"));

    node->addChild(parse_declaration_part());

    node->addChild(parse_compound_statement());

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_formal_parameter_list(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<formal-parameter-list>"));

    const Token& lparentToken = ts.expect(TokenType::LPARENT, "<formal-parameter-list>");
    node->addToken(lparentToken);

    node->addChild(parse_parameter_group());

    while (ts.check(TokenType::SEMICOLON)){
        const Token& semicolonToken = ts.expect(TokenType::SEMICOLON, "<formal-parameter-list>");
        node->addToken(semicolonToken);

        node->addChild(parse_parameter_group());
    }

    const Token& rparentToken = ts.expect(TokenType::RPARENT, "<formal-parameter-list>");
    node->addToken(rparentToken);

    return node;
}

std::unique_ptr<ParseTreeNode> Parser::parse_parameter_group(){
    auto node = std::unique_ptr<ParseTreeNode>(new ParseTreeNode("<parameter-group>"));
    
    node->addChild(parse_identifier_list());

    const Token& colonToken = ts.expect(TokenType::COLON, "<parameter-group>");
    node->addToken(colonToken);

    if (ts.check(TokenType::ARRAYSY)){
        node->addChild(parse_array_type());
    }
    else if (ts.check(TokenType::IDENT)){
        const Token& identToken = ts.expect(TokenType::IDENT, "<parameter-group>");
        node->addToken(identToken);
    }

    return node;
}


