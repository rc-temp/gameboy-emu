# Specify compiler
CXX = g++
# CXXFLAGS = -std=c++17 -Wall
CXXFLAGS = -std=c++20 -Wall

LDFLAGS = $(shell pkg-config --libs sdl2)
CXXFLAGS += $(shell pkg-config --cflags sdl2)

# Define directories
SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build/bin

# Define files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/gameboy-emu

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Default target
all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) $(LDFLAGS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
