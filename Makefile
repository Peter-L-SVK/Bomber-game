# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lncurses

# Debug configuration
DEBUG ?= 0
ifeq ($(DEBUG),1)
    CFLAGS += -DDEBUG -g -O0
else
    CFLAGS += -O2
endif

# Source files
SRC = bomber.c lib.c
OBJ = $(SRC:.c=.o)
HEADERS = bomber.h
TARGET = bomber

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile .c files to .o files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin

# Run the game
run: $(TARGET)
	./$(TARGET)

# Debug build
debug:
	$(MAKE) DEBUG=1

# Run with debug mode
run-debug: debug
	./$(TARGET)

# Phony targets
.PHONY: all clean install run debug run-debug
