# MemGuard Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -I./include
LDFLAGS = 

# Directories
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLES_DIR = examples
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/memguard.c
OBJECTS = $(BUILD_DIR)/memguard.o

# Targets
all: $(BUILD_DIR) test_basic

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile memguard.c
$(BUILD_DIR)/memguard.o: $(SRC_DIR)/memguard.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build test program
test_basic: $(OBJECTS) $(EXAMPLES_DIR)/test_basic.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test_basic.exe $(EXAMPLES_DIR)/test_basic.c $(OBJECTS)

# Run test
run: test_basic
	$(BUILD_DIR)/test_basic.exe

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Help
help:
	@echo "MemGuard Build System"
	@echo "====================="
	@echo "make          - Build all"
	@echo "make run      - Build and run test"
	@echo "make clean    - Remove build files"
	@echo "make help     - Show this help"

.PHONY: all run clean help
