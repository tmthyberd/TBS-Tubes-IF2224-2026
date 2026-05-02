#ifndef TREE_PRINTER_HPP
#define TREE_PRINTER_HPP

#include <iosfwd>
#include <string>
#include "ParseTreeNode.hpp"

class TreePrinter {
public:
    static void printToConsole(const ParseTreeNode& root);
    static void printToFile(const ParseTreeNode& root, const std::string& outputPath);
    static void print(const ParseTreeNode& root, std::ostream& out);
    static std::string toString(const ParseTreeNode& root);

private:
    static void printRecursive(const ParseTreeNode& node,
                               const std::string& prefix,
                               bool isLast,
                               std::ostream& out);
};

#endif
