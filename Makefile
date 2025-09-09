CXX = g++
CXXFLAGS = -O2 -Wall -Iinclude `root-config --cflags`
LDFLAGS = `root-config --libs`
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN = tmd

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

test: test_load_tables
	./test_load_tables

test_load_tables: tests/test_load_tables.cpp src/Table.cpp
	$(CXX) $(CXXFLAGS) $^ -o test_load_tables $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean