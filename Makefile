# Compiler and flags
CXX = g++
CXXFLAGS = -g -Wall -Wextra -std=c++17 -I./include -I/usr/include/SDL2

# SDL2 libraries
CLIBS = -lSDL2 -lSDL2_ttf -lSDL2_image

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Project name
TARGET = map_sketcher

# Find all source files in SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Convert source files to object files in BUILD_DIR
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
.PHONY: all
all: $(BIN_DIR)/$(TARGET)

# Building the executable
$(BIN_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(CLIBS)

# Compiling source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleaning up
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR) $(TARGET).o

# Rebuild the project
.PHONY: rebuild
rebuild: clean all
