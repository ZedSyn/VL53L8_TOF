# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g
LDFLAGS = 

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/my_project

# Default target: Build the executable
all: $(TARGET)

# Compile each .c file into an .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean compiled files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets (not real files)
.PHONY: all clean
