CXX = g++
CXXFLAGS = -O2 -Wall -Iinclude -Wno-deprecated-declarations `root-config --cflags`
LDFLAGS = `root-config --libs`

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
BIN = $(BIN_DIR)/tmd

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(BIN)

$(BIN): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# ----------------
# Tests
# ----------------
TESTS = $(BIN_DIR)/test_load_tables $(BIN_DIR)/test_grids

test: $(TESTS)
	./$(BIN_DIR)/test_load_tables
	./$(BIN_DIR)/test_grids

$(BIN_DIR)/test_load_tables: tests/test_load_tables.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_grids: tests/test_grids.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# ----------------
# Cleanup
# ----------------
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean test
