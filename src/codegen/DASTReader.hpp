#ifndef DAST_READER_HPP
#define DAST_READER_HPP

#include <memory>
#include <string>

#include "../semantic/ast/ASTNode.hpp"
#include "../semantic/symbol_table/Symbol_Table.hpp"

std::unique_ptr<ASTNode> load_decorated_ast(const std::string &text, SymbolTable &sym);

#endif
