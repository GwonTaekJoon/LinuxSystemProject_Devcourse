# Compiler and Compiler Flags
CC = gcc
CFLAGS = -Wall -g -Iui -Iweb_server -Isystem

# Source files and Object files
SRC = main.c ui/gui.c ui/input.c web_server/web_server.c system/system_server.c
OBJ = $(SRC:.c=.o)

# Target executable
TARGET_DIR=./bin/
TARGET = $(TARGET_DIR)toy_system

# Default target
all: $(TARGET)

# Rule to link object files to create the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target for removing compiled files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: all clean

