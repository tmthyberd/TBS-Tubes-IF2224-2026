#include "TreePrinter.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// Public Output Helpers

void TreePrinter::printToConsole(const ParseTreeNode& root) {
    print(root, std::cout);
}

void TreePrinter::printToFile(const ParseTreeNode& root, const std::string& outputPath) {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open parse tree output file '" + outputPath + "'");
    }

    print(root, file);
}

void TreePrinter::print(const ParseTreeNode& root, std::ostream& out) {
    out << root.name << '\n';

    for (std::size_t i = 0; i < root.children.size(); ++i) {
        printRecursive(*root.children[i], "", i + 1 == root.children.size(), out);
    }
}

std::string TreePrinter::toString(const ParseTreeNode& root) {
    std::ostringstream out;
    print(root, out);
    return out.str();
}

// Recursive Tree Traversal

void TreePrinter::printRecursive(const ParseTreeNode& node,
                                 const std::string& prefix,
                                 bool isLast,
                                 std::ostream& out) {
    out << prefix << (isLast ? "└── " : "├── ") << node.name << '\n';

    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        printRecursive(*node.children[i], childPrefix, i + 1 == node.children.size(), out);
    }
}
