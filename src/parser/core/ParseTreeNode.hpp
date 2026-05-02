#ifndef PARSE_TREE_NODE_HPP
#define PARSE_TREE_NODE_HPP

#include <memory>
#include <string>
#include <vector>
#include "../../lexer/token.h"

class ParseTreeNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ParseTreeNode> > children;

    explicit ParseTreeNode(const std::string& nodeName);

    ParseTreeNode(const ParseTreeNode&) = delete;
    ParseTreeNode& operator=(const ParseTreeNode&) = delete;
    ParseTreeNode(ParseTreeNode&&) = default;
    ParseTreeNode& operator=(ParseTreeNode&&) = default;

    ParseTreeNode& addChild(std::unique_ptr<ParseTreeNode> child);
    ParseTreeNode& addChild(const std::string& childName);
    ParseTreeNode& addToken(const Token& token);

private:
    static std::string tokenNodeName(const Token& token);
};

#endif
