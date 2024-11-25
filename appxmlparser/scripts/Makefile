# Variables
# csh C compiler for csh
# CC = cc
# C compiler options for csh
# CFLAGS = -xc99 -O2
CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -O2
TARGET = xmlparser
SRC_DIR ?= src
SRC = $(SRC_DIR)/xmlparser.c
BIN_DIR = bin

# Default target
all: $(BIN_DIR)/$(TARGET)

# Create bin directory if it doesn't exist and compile the source file into the target executable
$(BIN_DIR)/$(TARGET): $(SRC)
    $(CC) $(CFLAGS) -o $(BIN_DIR)/$(TARGET) $(SRC)

# Clean up the build directory
clean:
    rm -f $(BIN_DIR)/$(TARGET)

# Phony targets
.PHONY: all clean
