# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lncurses

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

# Phony targets
.PHONY: all clean install run
