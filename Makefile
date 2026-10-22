# ==============================================================================
# Makefile: Production-Grade Multi-Module ANSI C Build Tooling
# Course  : CS-115 Computer Programming
# ==============================================================================

CC        := gcc
CFLAGS    := -Wall -Wextra -Wpedantic -std=c99 -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS   := 

SRC_DIR   := src
INC_DIR   := include
OBJ_DIR   := obj
BIN_DIR   := bin

TARGET    := $(BIN_DIR)/blood_bank

SRCS      := $(SRC_DIR)/main.c \
             $(SRC_DIR)/donor.c \
             $(SRC_DIR)/inventory.c \
             $(SRC_DIR)/utils.c

OBJS      := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run setup

all: setup $(TARGET)

setup:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	@echo "Linking executable: $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build successful! Executable is ready at: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling module: $<"
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	@./$(TARGET)

clean:
	@echo "Cleaning compiled binaries and artifacts..."
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.tmp
	@echo "Repository clean."