CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
DEBUG_FLAGS = -DDEBUG_MODE

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET = $(BIN_DIR)/liballocator.a

all: directories $(TARGET)

directories: $(BUILD_DIR) $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJECTS)
	ar rcs $@ $^
	@echo "Built allocator library: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(SRC_DIR) -c $< -o $@

debug: clean directories
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -I$(INCLUDE_DIR) -I$(SRC_DIR) $(SOURCES) -o $(BIN_DIR)/allocator_test
	@echo "Built debug version"

test: all
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(SRC_DIR) -c $(SRC_DIR)/test_allocator.c -o $(BUILD_DIR)/test_allocator.o
	$(CC) $(CFLAGS) $(BUILD_DIR)/test_allocator.o $(TARGET) -o $(BIN_DIR)/allocator_test
	$(BIN_DIR)/allocator_test

benchmark: all
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(SRC_DIR) -c $(SRC_DIR)/benchmark.c -o $(BUILD_DIR)/benchmark.o
	$(CC) $(CFLAGS) $(BUILD_DIR)/benchmark.o $(TARGET) -o $(BIN_DIR)/benchmark
	$(BIN_DIR)/benchmark

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f benchmark_results.json

.PHONY: all debug clean directories test benchmark