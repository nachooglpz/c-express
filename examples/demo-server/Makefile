# Demo Server Example for C-Express Framework
# ==========================================

CC := gcc
CFLAGS := -Wall -Wextra -g -std=c99

# Paths
FRAMEWORK_ROOT := ../..
LIB_DIR := $(FRAMEWORK_ROOT)/lib
INCLUDE_DIR := $(FRAMEWORK_ROOT)

# Target
TARGET := demo-server

# Source
SRC := main.c

# Build
$(TARGET): $(SRC)
	@echo "Building demo server..."
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $< -L$(LIB_DIR) -lc-express

clean:
	rm -f $(TARGET)

.PHONY: clean
