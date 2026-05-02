#include "ParseTreeNode.hpp"
#include <sstream>

// Token Formatting Helper

namespace {
bool tokenTypeCarriesValue(TokenType type) {
    switch (type) {
        case TokenType::INTCON:
        case TokenType::REALCON:
        case TokenType::CHARCON:
        case TokenType::STRING:
        case TokenType::IDENT:
        case TokenType::COMMENT:
        case TokenType::UNKNOWN:
        case TokenType::ERROR:
            return true;
        default:
            return false;
    }
}
}

// Constructor

ParseTreeNode::ParseTreeNode(const std::string& nodeName)
    : name(nodeName), children() {}

std::string ParseTreeNode::tokenNodeName(const Token& token) {
    std::ostringstream out;
    out << tokenTypeToString(token.type);

    if (tokenTypeCarriesValue(token.type)) {
        out << "(" << token.value << ")";
    }

    return out.str();
}

// Child Management

ParseTreeNode& ParseTreeNode::addChild(std::unique_ptr<ParseTreeNode> child) {
    children.push_back(std::move(child));
    return *children.back();
}

ParseTreeNode& ParseTreeNode::addChild(const std::string& childName) {
    return addChild(std::unique_ptr<ParseTreeNode>(new ParseTreeNode(childName)));
}

ParseTreeNode& ParseTreeNode::addToken(const Token& token) {
    return addChild(tokenNodeName(token));
}
