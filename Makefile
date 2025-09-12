CXX = g++
# Dependency flags (only used when compiling .o files so .d files aren't generated during link steps)
DEPFLAGS = -MMD -MP
CXXFLAGS = -O2 -Wall -Iinclude -Wno-deprecated-declarations `root-config --cflags` -I$(HOME)/.local/include
LDFLAGS = `root-config --libs` -L$(HOME)/.local/lib64 -lyaml-cpp

SRC_DIR = src
MACRO_DIR = macro
TEST_DIR = tests
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Source/object files
SOURCES   := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS   := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS      := $(OBJECTS:.o=.d)

# Macros (main binaries)
MACRO_SOURCES := $(wildcard $(MACRO_DIR)/*.cpp)
MACRO_BINS    := $(MACRO_SOURCES:$(MACRO_DIR)/%.cpp=$(BIN_DIR)/%)

# Tests
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS    := $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(BIN_DIR)/%)

# ----------------
# Default targets
# ----------------
all: $(MACRO_BINS) $(TEST_BINS)

tests: $(TEST_BINS)

# Convenience: build + run all tests with default args
run-tests: $(TEST_BINS)
	./$(BIN_DIR)/test_load_tables
	./$(BIN_DIR)/test_grids
	./$(BIN_DIR)/test_1D_plots --file out/output.root --tree tree --energy 0x0
	./$(BIN_DIR)/test_injectExtract --file out/output.root --tree tree --energy default --n_injections 5 --bin_index 0 --A_opt 0.3 --outDir out --outFilename test_injectExtract.yaml
	./$(BIN_DIR)/test_2D_plots --file out/output.root --tree tree --energy 0x0

# ----------------
# Build rules
# ----------------

# Rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INC_DIR) -c $< -o $@

# Rule for macros (link macro .cpp with shared objects)
$(BIN_DIR)/%: $(MACRO_DIR)/%.cpp $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(OBJECTS) -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

# Rule for tests (link test .cpp with shared objects)
$(BIN_DIR)/%: $(TEST_DIR)/%.cpp $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(OBJECTS) -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

# ----------------
# Cleanup
# ----------------
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean tests run-tests

-include $(DEPS)
