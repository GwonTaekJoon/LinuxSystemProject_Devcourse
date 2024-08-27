# Compiler and Compiler Flags

BUILDROOT_DIR = /home/gwontaekjoon/Desktop/buildroot
TOOLCHAIN_DIR = $(BUILDROOT_DIR)/output/host/bin
CC = $(TOOLCHAIN_DIR)/aarch64-buildroot-linux-gnu-gcc
CXX = $(TOOLCHAIN_DIR)/aarch64-buildroot-linux-gnu-g++
CFLAGS = -Wall -g -Iui -Iweb_server -Isystem -Ihal -I./ -fPIC -Ihal/include
CXXFLAGS = $(CFLAGS) -std=c++14
CXXLIBS = -lpthread -lm -lrt -ldl -lseccomp
LDFLAGS = -Wl,--no-as-needed

# Source files and Object files
CSRC = main.c ui/gui.c ui/input.c web_server/web_server.c \
       system/system_server.c system/shared_memory.c system/dump_state.c \
       hal/hardware.c
CXXSRC = hal/oem/camera_HAL_oem.cpp hal/oem/ControlThread.cpp \
         hal/toy/camera_HAL_toy.cpp hal/toy/ControlThread.cpp
COBJ = $(CSRC:.c=.o)
CXXOBJ = $(CXXSRC:.cpp=.o)

OBJ = $(COBJ)

# Target executable
TARGET_DIR = ./bin/
TARGET = $(TARGET_DIR)toy_system

# Default target
all: $(TARGET) libcamera.oem.so libcamera.toy.so

# Rule to link object files to create the executable
$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXXLIBS) $(LDFLAGS)
	make modules

# Rule to compile C source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile C++ source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build libcamera.oem.so
libcamera.oem.so: hal/oem/camera_HAL_oem.o hal/oem/ControlThread.o
	$(CXX) -shared -fPIC -o $@ $^ $(CXXLIBS) $(LDFLAGS)

# Rule to build libcamera.toy.so
libcamera.toy.so: hal/toy/camera_HAL_toy.o hal/toy/ControlThread.o
	$(CXX) -shared -fPIC -o $@ $^ $(CXXLIBS) $(LDFLAGS)

# Clean target for removing compiled files
clean:
	rm -f $(COBJ) $(CXXOBJ) $(TARGET) libcamera.oem.so libcamera.toy.so\
		bin/filebrowser.db
	make -C drivers/simple_io clean
	make -C drivers/engine clean
	make -C drivers/state_machine clean

# Phony targets
.PHONY: all clean libcamera.oem.so libcamera.toy.so

.PHONY: modules
modules:
	cd drivers/simple_io && make
	cd drivers/engine && make
	cd drivers/state_machine && make

