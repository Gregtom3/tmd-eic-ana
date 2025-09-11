CXX = g++
CXXFLAGS = -O2 -Wall -Iinclude -Wno-deprecated-declarations -MMD -MP `root-config --cflags`
LDFLAGS = `root-config --libs`

SRC_DIR = src
MACRO_DIR = macro
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

MACRO_SOURCES := $(wildcard $(MACRO_DIR)/*.cpp)
MACRO_BINS := $(MACRO_SOURCES:$(MACRO_DIR)/%.cpp=$(BIN_DIR)/%)

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)


all: $(MACRO_BINS)

$(BIN_DIR)/%: $(MACRO_DIR)/%.cpp $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(OBJECTS) -o $@ $(LDFLAGS) -lRooFit -lRooFitCore -MF $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

-include $(DEPS)

# ----------------
# Tests
# ----------------
TESTS = $(BIN_DIR)/test_load_tables $(BIN_DIR)/test_grids $(BIN_DIR)/test_generate_and_use_tree

test: $(TESTS)
	./$(BIN_DIR)/test_load_tables
	./$(BIN_DIR)/test_grids
	./$(BIN_DIR)/test_generate_and_use_tree

$(BIN_DIR)/test_load_tables: tests/test_load_tables.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

$(BIN_DIR)/test_grids: tests/test_grids.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_generate_and_use_tree: tests/test_generate_and_use_tree.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp src/TMD.cpp src/Hist.cpp src/Inject.cpp src/Plotter.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

# ----------------
# Cleanup
# ----------------
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean test
