CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -I/opt/homebrew/opt/googletest/include
LDFLAGS = -L/opt/homebrew/opt/googletest/lib -lgtest -lgtest_main -pthread -lexpat

SRC_DIR = src
TEST_DIR = testsrc
OBJ_DIR = obj
BIN_DIR = bin

# Sources
SRC = $(wildcard $(SRC_DIR)/*.cpp)
#TEST = $(wildcard $(TEST_DIR)/*.cpp)
TEST = testsrc/StringDataSinkTest.cpp testsrc/StringDataSourceTest.cpp testsrc/CSVBusSystem.cpp testsrc/OpenStreetMapTest.cpp

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
TEST_OBJ_FILES = $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST))

GTEST_TARGET = $(BIN_DIR)/runtests

all: $(GTEST_TARGET)

$(GTEST_TARGET): $(OBJ_FILES) $(TEST_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

test: all