CXX = g++
# Dependency flags: only used when compiling .o files so .d files are not generated during link steps
DEPFLAGS = -MMD -MP
CXXFLAGS = -O2 -Wall -Iinclude -Wno-deprecated-declarations `root-config --cflags` -I$(HOME)/.local/include
LDFLAGS = `root-config --libs` -L$(HOME)/.local/lib64 -lyaml-cpp

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
	$(CXX) $(CXXFLAGS) $< $(OBJECTS) -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INC_DIR) -c $< -o $@

-include $(DEPS)

# ----------------
# Tests
# ----------------
TESTS = $(BIN_DIR)/test_load_tables $(BIN_DIR)/test_grids $(BIN_DIR)/test_fillHistograms $(BIN_DIR)/test_injectExtract

test: $(TESTS)
	./$(BIN_DIR)/test_load_tables
	./$(BIN_DIR)/test_grids
	./$(BIN_DIR)/test_fillHistograms
	./$(BIN_DIR)/test_injectExtract out/output_generated.root tree default --n_injections 5 --bin_index 0 --A_opt 0.3 --out out --outFilename test_injectExtract.yaml

$(BIN_DIR)/test_load_tables: tests/test_load_tables.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

$(BIN_DIR)/test_grids: tests/test_grids.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_fillHistograms: tests/test_fillHistograms.cpp src/Hist.cpp src/Plotter.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp src/Inject.cpp src/TMD.cpp src/InjectionProject.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

$(BIN_DIR)/test_injectExtract: tests/test_injectExtract.cpp src/Hist.cpp src/Plotter.cpp src/Table.cpp src/Grid.cpp src/Bin.cpp src/Inject.cpp src/TMD.cpp src/InjectionProject.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lRooFit -lRooFitCore

# ----------------
# Cleanup
# ----------------
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean test
