# Compiler and flags
CC = gcc
CFLAGS = -IPlatform -IVL53L8CX_ULD_API/inc IHapticMotor -Wall -Wextra -g
LDFLAGS = -lwiringPi

# Directories
PLATFORM_DIR = Platform
VL53L8CX_DIR = VL53L8CX_ULD_API
HAPTICMOTOR_DIR = HapticMotor
BUILD_DIR = build

# Source and object files
MAIN_SRC = main.c
PLATFORM_SRCS = $(wildcard $(PLATFORM_DIR)/*.c)
VL53L8CX_SRCS = $(wildcard $(VL53L8CX_DIR)/src/*.c)
HAPTICMOTOR_SRCS = $(wildcard $(HAPTICMOTOR_DIR)/*.c)
SRCS = $(MAIN_SRC) $(PLATFORM_SRCS) $(VL53L8CX_SRCS) $(HAPTICMOTOR_SRCS)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/my_project

# Default target: Build the executable
all: $(TARGET)

# Compile each .c file into an .o file
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)  # Ensure subdirectories exist
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
