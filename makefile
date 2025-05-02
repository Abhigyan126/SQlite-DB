# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -g

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Executable name
TARGET = build/main

# Default target
all: $(TARGET)

# Create object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(TARGET)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Run the executable
run: $(TARGET)
	./$(TARGET)

# Clean up object files and executable
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all run clean
