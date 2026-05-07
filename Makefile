CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
 
 
LEXER_TARGET = lexer
PARSER_TARGET = parser
 
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
 
LEXER_HEADERS = src/lexer/token.h \
                src/lexer/lexer.h
 
PARSER_CORE_HEADERS = src/parser/core/TokenStream.hpp \
                      src/parser/core/ParseTreeNode.hpp \
                      src/parser/core/TreePrinter.hpp \
                      src/parser/core/ErrorHandler.hpp
 
HEADERS = $(LEXER_HEADERS) $(PARSER_CORE_HEADERS)
 
# Object files
LEXER_OBJS = $(LEXER_SRCS:.cpp=.o)
PARSER_OBJS = $(PARSER_SRCS:.cpp=.o)
 
# Detect OS for clean command
ifeq ($(OS),Windows_NT)
    RM = del /Q
    RMDIR = rmdir /Q /S
    LEXER_EXE = $(LEXER_TARGET).exe
    PARSER_EXE = $(PARSER_TARGET).exe
else
    RM = rm -f
    RMDIR = rm -rf
    LEXER_EXE = $(LEXER_TARGET)
    PARSER_EXE = $(PARSER_TARGET)
endif
 
all: $(LEXER_TARGET) $(PARSER_TARGET)
 
# Linking lexer
$(LEXER_TARGET): $(LEXER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(LEXER_TARGET) $(LEXER_OBJS)
 
# Linking parser
$(PARSER_TARGET): $(PARSER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(PARSER_TARGET) $(PARSER_OBJS)
 
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
	-$(RM) $(LEXER_EXE) 2>nul
	-$(RM) $(PARSER_EXE) 2>nul
else
	$(RM) $(LEXER_OBJS) $(PARSER_OBJS) $(LEXER_TARGET) $(PARSER_TARGET)
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
 
# Alias eksplisit tanpa tanda hubung.
runparser: run-parser
 
# Alias eksplisit tanpa tanda hubung.
runlexer: run-lexer
 
# Phony targets
.PHONY: all clean debug run-lexer runlexer run-parser runparser