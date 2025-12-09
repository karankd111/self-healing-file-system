CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include

# Detect OS and set appropriate flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lstdc++fs
else ifeq ($(UNAME_S),Darwin)
    # macOS - filesystem is built into libc++
    LDFLAGS = 
    CXX = clang++
endif

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

SOURCES = $(SRC_DIR)/block.cpp $(SRC_DIR)/storage.cpp $(SRC_DIR)/recovery.cpp $(SRC_DIR)/filesystem.cpp $(SRC_DIR)/main.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/shfs

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/* data/fs_storage

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run