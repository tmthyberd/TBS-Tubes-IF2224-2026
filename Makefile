CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic


LEXER_TARGET = lexer
PARSER_TARGET = parser

# Source files
LEXER_SRCS = src/main.cpp \
             src/lexer/lexer.cpp

PARSER_CORE_SRCS = src/parser/core/TokenStream.cpp \
                   src/parser/core/ParseTreeNode.cpp \
                   src/parser/core/TreePrinter.cpp \
                   src/parser/core/ErrorHandler.cpp

LEXER_HEADERS = src/lexer/token.h \
                src/lexer/lexer.h

PARSER_CORE_HEADERS = src/parser/core/TokenStream.hpp \
                      src/parser/core/ParseTreeNode.hpp \
                      src/parser/core/TreePrinter.hpp \
                      src/parser/core/ErrorHandler.hpp

HEADERS = $(LEXER_HEADERS) $(PARSER_CORE_HEADERS)

# Object files
LEXER_OBJS = $(LEXER_SRCS:.cpp=.o)
PARSER_CORE_OBJS = $(PARSER_CORE_SRCS:.cpp=.o)

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

all: $(LEXER_TARGET)

# Linking lexer
$(LEXER_TARGET): $(LEXER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(LEXER_TARGET) $(LEXER_OBJS)

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
	-$(RM) src\parser\core\*.o 2>nul
	-$(RM) $(LEXER_EXE) 2>nul
	-$(RM) $(PARSER_EXE) 2>nul
else
	$(RM) $(LEXER_OBJS) $(PARSER_CORE_OBJS) $(LEXER_TARGET) $(PARSER_TARGET)
endif

# Run lexer dengan file test (jika ada)
run-lexer: all
ifeq ($(OS),Windows_NT)
	$(LEXER_EXE) test\lexer\input-1.txt
else
	./$(LEXER_TARGET) test/lexer/input-1.txt
endif

# Run parser. Parser entry point belum ada sampai modul B/C/D diintegrasikan.
run-parser:
	@echo Belum diimplementasi

# Alias eksplisit tanpa tanda hubung.
runparser: run-parser

# Alias eksplisit tanpa tanda hubung.
runlexer: run-lexer

# Phony targets
.PHONY: all clean debug run-lexer runlexer run-parser runparser
