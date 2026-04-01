CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
TARGET = lexer

SRC = src/main.cpp src/lexer.cpp src/token.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)