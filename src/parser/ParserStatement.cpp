#include "Parser.hpp"
#include "ErrorHandler.hpp"
using namespace std;

unique_ptr<ParseTreeNode> Parser::parse_compound_statement()
{
    auto node = make_unique<ParseTreeNode>("<compound-statement>");
    node->addToken(ts.expect(TokenType::BEGINSY, "<compound-statement>"));
    node->addChild(parse_statement_list());
    node->addToken(ts.expect(TokenType::ENDSY, "<compound-statement>"));
    return node;
}

unique_ptr<ParseTreeNode> Parser::parse_statement_list()
{
    auto node = make_unique<ParseTreeNode>("<statement-list>");

    node->addChild(parse_statement());

    while (ts.check(TokenType::SEMICOLON))
    {
        TokenType after = ts.peek(1).type;

        if (after == TokenType::ENDSY || after == TokenType::UNTILSY)
        {
            break;
        }

        node->addToken(ts.advance());
        node->addChild(parse_statement());
    }
    return node;
}

static bool isStatementStart(const Token &t)
{
    switch (t.type)
    {
    case TokenType::IDENT:
    case TokenType::BEGINSY:
    case TokenType::IFSY:
    case TokenType::CASESY:
    case TokenType::WHILESY:
    case TokenType::REPEATSY:
    case TokenType::FORSY:
        return true;
    default:
        return false;
    }
}
unique_ptr<ParseTreeNode> Parser::parse_statement()
{
    auto node = make_unique<ParseTreeNode>("<statement>");

    if (ts.check(TokenType::BEGINSY))
    {
        node->addChild(parse_compound_statement());
    }
    else if (ts.check(TokenType::IFSY))
    {
        node->addChild(parse_if_statement());
    }
    else if (ts.check(TokenType::CASESY))
    {
        node->addChild(parse_case_statement());
    }
    else if (ts.check(TokenType::WHILESY))
    {
        node->addChild(parse_while_statement());
    }
    else if (ts.check(TokenType::REPEATSY))
    {
        node->addChild(parse_repeat_statement());
    }
    else if (ts.check(TokenType::FORSY))
    {
        node->addChild(parse_for_statement());
    }
    else if (ts.check(TokenType::IDENT))
    {
        bool isAssignment = false;
        std::size_t offset = 1;

        while (true)
        {
            TokenType t = ts.peek(offset).type;

            if (t == TokenType::LBRACK)
            {
                offset++;
                int depth = 1;

                while (depth > 0 && ts.peek(offset).type != TokenType::END_OF_FILE)
                {
                    if (ts.peek(offset).type == TokenType::LBRACK)
                        depth++;
                    else if (ts.peek(offset).type == TokenType::RBRACK)
                        depth--;

                    offset++;
                }
            }
            else if (t == TokenType::PERIOD)
            {
                offset++;
                offset++;
            }
            else
            {
                break;
            }
        }

        isAssignment = (ts.peek(offset).type == TokenType::BECOMES);

        if (isAssignment)
        {
            node->addChild(parse_assignment_statement());
        }
        else
        {
            node->addChild(parse_procedure_function_call());
        }
    }
    return node;
}
unique_ptr<ParseTreeNode> Parser::parse_assignment_statement()
{
    auto node = make_unique<ParseTreeNode>("<assignment-statement>");
    node->addChild(parse_variable());
    node->addToken(ts.expect(TokenType::BECOMES, "<assignment-statement>"));
    node->addChild(parse_expression());
    return node;
}
unique_ptr<ParseTreeNode> Parser::parse_if_statement()
{
    auto node = make_unique<ParseTreeNode>("<if-statement>");

    node->addToken(ts.expect(TokenType::IFSY, "<if-statement>"));
    node->addChild(parse_expression());
    node->addToken(ts.expect(TokenType::THENSY, "<if-statement>"));
    node->addChild(parse_statement());

    if (ts.check(TokenType::ELSESY))
    {
        node->addToken(ts.advance());
        node->addChild(parse_statement());
    }
    return node;
}
unique_ptr<ParseTreeNode> Parser::parse_case_statement()
{
    auto node = make_unique<ParseTreeNode>("<case-statement>");

    node->addToken(ts.expect(TokenType::CASESY, "<case-statement>"));
    node->addChild(parse_expression());
    node->addToken(ts.expect(TokenType::OFSY, "<case-statement>"));
    node->addChild(parse_case_block());
    node->addToken(ts.expect(TokenType::ENDSY, "<case-statement>"));
    return node;
}

static bool isCaseConstantStart(const Token &t)
{
    switch (t.type)
    {
    case TokenType::CHARCON:
    case TokenType::STRING:
    case TokenType::INTCON:
    case TokenType::REALCON:
    case TokenType::IDENT:
    case TokenType::PLUS:
    case TokenType::MINUS:
        return true;
    default:
        return false;
    }
}

unique_ptr<ParseTreeNode> Parser::parse_case_block()
{
    auto node = make_unique<ParseTreeNode>("<case-block>");

    // constant list
    node->addChild(parse_constant());
    while (ts.check(TokenType::COMMA))
    {
        node->addToken(ts.advance());
        node->addChild(parse_constant());
    }
    node->addToken(ts.expect(TokenType::COLON, "<case-block>"));
    node->addChild(parse_statement());

    // (semicolon case-block?)*
    while (ts.check(TokenType::SEMICOLON))
    {
        // peek past the semicolon
        if (!isCaseConstantStart(ts.peek(1)))
            break;                    // trailing semicolon before 'end'
        node->addToken(ts.advance()); // semicolon
        node->addChild(parse_case_block());
    }
    return node;
}
unique_ptr<ParseTreeNode> Parser::parse_while_statement()
{
    auto node = make_unique<ParseTreeNode>("<while-statement>");
    node->addToken(ts.expect(TokenType::WHILESY, "<while-statement>"));
    node->addChild(parse_expression());
    node->addToken(ts.expect(TokenType::DOSY, "<while-statement>"));
    node->addChild(parse_statement());
    return node;
}

unique_ptr<ParseTreeNode> Parser::parse_repeat_statement()
{
    auto node = std::make_unique<ParseTreeNode>("<repeat-statement>");
    node->addToken(ts.expect(TokenType::REPEATSY, "<repeat-statement>"));
    node->addChild(parse_statement_list());
    node->addToken(ts.expect(TokenType::UNTILSY, "<repeat-statement>"));
    node->addChild(parse_expression());
    return node;
}

unique_ptr<ParseTreeNode> Parser::parse_for_statement()
{
    auto node = std::make_unique<ParseTreeNode>("<for-statement>");
    node->addToken(ts.expect(TokenType::FORSY, "<for-statement>"));
    node->addToken(ts.expect(TokenType::IDENT, "<for-statement>"));
    node->addToken(ts.expect(TokenType::BECOMES, "<for-statement>"));
    node->addChild(parse_expression());

    if (ts.check(TokenType::TOSY) || ts.check(TokenType::DOWNTOSY))
        node->addToken(ts.advance());
    else
        ErrorHandler::unexpectedToken(ts.peek(), "tosy or downtosy", "<for-statement>");

    node->addChild(parse_expression());
    node->addToken(ts.expect(TokenType::DOSY, "<for-statement>"));
    node->addChild(parse_statement());
    return node;
}
unique_ptr<ParseTreeNode> Parser::parse_procedure_function_call()
{
    auto node = std::make_unique<ParseTreeNode>("<procedure/function-call>");
    node->addToken(ts.expect(TokenType::IDENT, "<procedure/function-call>"));
    if (ts.check(TokenType::LPARENT))
    {
        node->addToken(ts.advance()); // lparent
        node->addChild(parse_parameter_list());
        node->addToken(ts.expect(TokenType::RPARENT, "<procedure/function-call>"));
    }
    return node;
}