CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic


LEXER_TARGET = lexer
PARSER_TARGET = parser
SEMANTIC_TARGET = semantic
COMPILER_TARGET = compiler

# Source files
LEXER_SRCS = src/main.cpp \
             src/lexer/lexer.cpp

PARSER_SRCS = src/parser_main.cpp \
              src/lexer/lexer.cpp \
              src/parser/core/TokenStream.cpp \
              src/parser/core/ParseTreeNode.cpp \
              src/parser/core/TreePrinter.cpp \
              src/parser/core/ErrorHandler.cpp \
              src/parser/ParserDeclaration.cpp \
              src/parser/ParserStatement.cpp \
              src/parser/ParserExpression.cpp

SEMANTIC_SRCS = src/semantic_main.cpp \
                src/parser/core/ParseTreeNode.cpp \
                src/semantic/symbol_table/SymbolTable.cpp \
                src/semantic/ASTBuilder.cpp \
                src/semantic/visitor/VisitorStatement.cpp

# Pipeline penuh: lexer -> parser -> semantic -> codegen -> VM.
# Catatan: src/codegen/CodegenStmt.cpp (Orang 2) dan src/codegen/CodegenExpr.cpp
# (Orang 3) wajib ada agar tahap linking compiler berhasil.
COMPILER_SRCS = src/compiler_main.cpp \
                src/lexer/lexer.cpp \
                src/parser/core/TokenStream.cpp \
                src/parser/core/ParseTreeNode.cpp \
                src/parser/core/ErrorHandler.cpp \
                src/parser/ParserDeclaration.cpp \
                src/parser/ParserStatement.cpp \
                src/parser/ParserExpression.cpp \
                src/semantic/symbol_table/SymbolTable.cpp \
                src/semantic/ASTBuilder.cpp \
                src/semantic/visitor/VisitorStatement.cpp \
                src/codegen/CodegenStmt.cpp \
                src/codegen/CodegenExpr.cpp \
                src/interpreter/VM_Memory.cpp \
                src/interpreter/VM_Exceptions.cpp \
                src/interpreter/VM_Operations.cpp

LEXER_HEADERS = src/lexer/token.h \
                src/lexer/lexer.h

PARSER_CORE_HEADERS = src/parser/core/TokenStream.hpp \
                      src/parser/core/ParseTreeNode.hpp \
                      src/parser/core/TreePrinter.hpp \
                      src/parser/core/ErrorHandler.hpp \
                      src/parser/Parser.hpp

SEMANTIC_HEADERS = src/semantic/ast/ASTNode.hpp \
                   src/semantic/ast/ExprNodes.hpp \
                   src/semantic/ast/StmtNodes.hpp \
                   src/semantic/ast/DeclNodes.hpp \
                   src/semantic/ASTBuilder.hpp \
                   src/semantic/symbol_table/Symbol_Table.hpp \
                   src/semantic/visitor/SemanticVisitor.hpp \
                   src/semantic/error/SemanticError.hpp

COMPILER_HEADERS = src/codegen/Instruction.hpp \
                   src/codegen/CodegenVisitor.hpp \
                   src/interpreter/VM_Value.hpp \
                   src/interpreter/VM_Memory.hpp \
                   src/interpreter/VM_Exceptions.hpp \
                   src/interpreter/VirtualMachine.hpp

HEADERS = $(LEXER_HEADERS) $(PARSER_CORE_HEADERS) $(SEMANTIC_HEADERS) $(COMPILER_HEADERS)

# Object files
LEXER_OBJS = $(LEXER_SRCS:.cpp=.o)
PARSER_OBJS = $(PARSER_SRCS:.cpp=.o)
SEMANTIC_OBJS = $(SEMANTIC_SRCS:.cpp=.o)
COMPILER_OBJS = $(COMPILER_SRCS:.cpp=.o)

# Detect OS for clean command
ifeq ($(OS),Windows_NT)
    RM = del /Q
    RMDIR = rmdir /Q /S
    LEXER_EXE = $(LEXER_TARGET).exe
    PARSER_EXE = $(PARSER_TARGET).exe
    SEMANTIC_EXE = $(SEMANTIC_TARGET).exe
    COMPILER_EXE = $(COMPILER_TARGET).exe
else
    RM = rm -f
    RMDIR = rm -rf
    LEXER_EXE = $(LEXER_TARGET)
    PARSER_EXE = $(PARSER_TARGET)
    SEMANTIC_EXE = $(SEMANTIC_TARGET)
    COMPILER_EXE = $(COMPILER_TARGET)
endif

all: $(LEXER_TARGET) $(PARSER_TARGET) $(SEMANTIC_TARGET) $(COMPILER_TARGET)

# Linking lexer
$(LEXER_TARGET): $(LEXER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(LEXER_TARGET) $(LEXER_OBJS)

# Linking parser
$(PARSER_TARGET): $(PARSER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(PARSER_TARGET) $(PARSER_OBJS)

# Linking semantic
$(SEMANTIC_TARGET): $(SEMANTIC_OBJS)
	$(CXX) $(CXXFLAGS) -o $(SEMANTIC_TARGET) $(SEMANTIC_OBJS)

# Linking compiler (pipeline penuh + Virtual Machine)
$(COMPILER_TARGET): $(COMPILER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(COMPILER_TARGET) $(COMPILER_OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build dengan debug symbols
debug: CXXFLAGS += -g -DDEBUG
debug: clean all

# Clean up
clean:
ifeq ($(OS),Windows_NT)
	-$(RM) *.o 2>nul
	-$(RM) src\*.o 2>nul
	-$(RM) src\lexer\*.o 2>nul
	-$(RM) src\parser\*.o 2>nul
	-$(RM) src\parser\core\*.o 2>nul
	-$(RM) src\semantic\*.o 2>nul
	-$(RM) src\semantic\ast\*.o 2>nul
	-$(RM) src\semantic\symbol_table\*.o 2>nul
	-$(RM) src\semantic\visitor\*.o 2>nul
	-$(RM) src\semantic\printer\*.o 2>nul
	-$(RM) src\codegen\*.o 2>nul
	-$(RM) src\interpreter\*.o 2>nul
	-$(RM) $(LEXER_EXE) 2>nul
	-$(RM) $(PARSER_EXE) 2>nul
	-$(RM) $(SEMANTIC_EXE) 2>nul
	-$(RM) $(COMPILER_EXE) 2>nul
else
	$(RM) $(LEXER_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(COMPILER_OBJS) $(LEXER_TARGET) $(PARSER_TARGET) $(SEMANTIC_TARGET) $(COMPILER_TARGET)
endif

# Run lexer dengan file test (jika ada)
run-lexer: $(LEXER_TARGET)
ifeq ($(OS),Windows_NT)
	$(LEXER_EXE) test\lexer\input-1.txt
else
	./$(LEXER_TARGET) test/lexer/input-1.txt
endif

# Run parser dengan file test (jika ada)
run-parser: $(PARSER_TARGET)
ifeq ($(OS),Windows_NT)
	$(LEXER_EXE) test\lexer\input-1.txt tokens.txt && $(PARSER_EXE) tokens.txt
else
	./$(LEXER_TARGET) test/lexer/input-1.txt tokens.txt && ./$(PARSER_TARGET) tokens.txt
endif

# Run semantic dengan file test (jika ada).
# Semantic menerima parse tree sebagai input, sehingga kita rangkai
# lexer -> parser -> semantic terlebih dulu untuk membangun parse tree.
run-semantic: $(LEXER_TARGET) $(PARSER_TARGET) $(SEMANTIC_TARGET)
ifeq ($(OS),Windows_NT)
	$(LEXER_EXE) test\semantic\input-tc1.txt tokens.tmp && $(PARSER_EXE) tokens.tmp parse_tree.tmp && $(SEMANTIC_EXE) parse_tree.tmp
else
	./$(LEXER_TARGET) test/semantic/input-tc1.txt tokens.tmp && ./$(PARSER_TARGET) tokens.tmp parse_tree.tmp && ./$(SEMANTIC_TARGET) parse_tree.tmp
endif

# Run compiler dengan source Arion. Pipeline penuh sampai eksekusi VM.
run-compiler: $(COMPILER_TARGET)
ifeq ($(OS),Windows_NT)
	$(COMPILER_EXE) test\codegen\input\input-tc1-assignment.txt
else
	./$(COMPILER_TARGET) test/codegen/input/input-tc1-assignment.txt
endif

# Alias eksplisit tanpa tanda hubung.
runparser: run-parser
runlexer: run-lexer
runsemantic: run-semantic
runcompiler: run-compiler

# Phony targets
.PHONY: all clean debug run-lexer runlexer run-parser runparser run-semantic runsemantic run-compiler runcompiler
