# Makefile for Merkle Tree Tests
# Author: Guy Alster
# Date: 2025-06-01

# Compiler and flags
# Compiler and flags
CC = clang
CFLAGS = -Werror -std=c11 -g -O0 -Wno-deprecated-declarations -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unused-parameter

# Use pkg-config if available to find OpenSSL
OPENSSL_CFLAGS ?= $(shell pkg-config --cflags openssl 2>/dev/null)
OPENSSL_LIBS  ?= $(shell pkg-config --libs openssl 2>/dev/null)

INCLUDES = -I../include $(OPENSSL_CFLAGS)
LDFLAGS = $(OPENSSL_LIBS)

# Source files
SRC_DIR = ../src
SOURCES = $(SRC_DIR)/merkle_tree.c $(SRC_DIR)/merkle_queue.c $(SRC_DIR)/merkle_utils.c
TEST_SOURCES = test_merkle_tree.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Executables
TEST_EXECUTABLE = test_merkle_tree

# Default target
all: $(TEST_EXECUTABLE)

# Build test executable
$(TEST_EXECUTABLE): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(OBJECTS) $(TEST_OBJECTS) -o $@ $(LDFLAGS)

# Compile source files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile test files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
test: $(TEST_EXECUTABLE)
	@echo "Running Merkle Tree Tests..."
	@echo "============================="
	@./$(TEST_EXECUTABLE)

# Run tests with memory checking (if valgrind is available)
test-memory: $(TEST_EXECUTABLE)
	@echo "Running tests with memory checking..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_EXECUTABLE); \
	else \
		echo "Valgrind not found, running tests normally..."; \
		./$(TEST_EXECUTABLE); \
	fi

# Run tests with debug output
test-debug: CFLAGS += -DMERKLE_DEBUG
test-debug: clean $(TEST_EXECUTABLE)
	@echo "Running tests with debug output..."
	@./$(TEST_EXECUTABLE)

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TEST_EXECUTABLE)
	rm -f *.o

# Clean and rebuild
rebuild: clean all

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build test executable"
	@echo "  test         - Build and run tests"
	@echo "  test-memory  - Run tests with memory checking (requires valgrind)"
	@echo "  test-debug   - Run tests with debug output enabled"
	@echo "  clean        - Remove build artifacts"
	@echo "  rebuild      - Clean and rebuild"
	@echo "  help         - Show this help message"

# Phony targets
.PHONY: all test test-memory test-debug clean rebuild help

# Dependencies (manual for now, could use gcc -MM to generate)
$(SRC_DIR)/merkle_tree.o: $(SRC_DIR)/merkle_tree.c ../include/Merkle.h ../include/MerkleQueue.h ../include/Utils.h
$(SRC_DIR)/merkle_queue.o: $(SRC_DIR)/merkle_queue.c ../include/MerkleQueue.h ../include/Utils.h
$(SRC_DIR)/merkle_utils.o: $(SRC_DIR)/merkle_utils.c ../include/Utils.h
test_merkle_tree.o: test_merkle_tree.c ../include/Merkle.h ../include/Utils.h