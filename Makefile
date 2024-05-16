# Compiler and Compiler Flags
CC = gcc
CXX = g++
CFLAGS = -Wall -g -Iui -Iweb_server -Isystem -Ihal -I./
CXXFLAGS = $(CFLAGS) -std=c++14
CXXLIBS = -lpthread -lm -lrt
# Source files and Object files
CSRC = main.c ui/gui.c ui/input.c web_server/web_server.c\
system/system_server.c system/shared_memory.c system/dump_state.c
CXXSRC = hal/camera_HAL.cpp hal/ControlThread.cpp
COBJ = $(CSRC:.c=.o)
CXXOBJ = $(CXXSRC:.cpp=.o)

OBJ = $(COBJ) $(CXXOBJ)


# Target executable
TARGET_DIR=./bin/
TARGET = $(TARGET_DIR)toy_system

# Default target
all: $(TARGET)

# Rule to link object files to create the executable
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(CXXLIBS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: $.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target for removing compiled files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: all clean

