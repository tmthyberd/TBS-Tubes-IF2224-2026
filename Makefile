CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic


TARGET = lexer

# Source files
SRCS = src/main.cpp src/lexer.cpp
HEADERS = src/token.h src/lexer.h

# Object files
OBJS = $(SRCS:.cpp=.o)

# Detect OS for clean command
ifeq ($(OS),Windows_NT)
    RM = del /Q
    RMDIR = rmdir /Q /S
    TARGET_EXE = $(TARGET).exe
else
    RM = rm -f
    RMDIR = rm -rf
    TARGET_EXE = $(TARGET)
endif

all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile main.cpp
main.o: src/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c src/main.cpp

# Compile lexer.cpp
lexer.o: src/lexer.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c src/lexer.cpp

# Build dengan debug symbols
debug: CXXFLAGS += -g -DDEBUG
debug: clean all

# Clean up
clean:
ifeq ($(OS),Windows_NT)
	-$(RM) *.o 2>nul
	-$(RM) $(TARGET_EXE) 2>nul
else
	$(RM) $(OBJS) $(TARGET)
endif

# Run dengan file test (jika ada)
run: all
ifeq ($(OS),Windows_NT)
	$(TARGET_EXE) ..\test\milestone-1\input-1.txt
else
	./$(TARGET) ../test/milestone-1/input-1.txt
endif

# Phony targets
.PHONY: all clean debug run
