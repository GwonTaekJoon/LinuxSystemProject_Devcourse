# Compiler and Compiler Flags

BUILDROOT_DIR = /home/gwontaekjoon/Desktop/buildroot
TOOLCHAIN_DIR = $(BUILDROOT_DIR)/output/host/bin
CC = $(TOOLCHAIN_DIR)/aarch64-buildroot-linux-gnu-gcc
CXX = $(TOOLCHAIN_DIR)/aarch64-buildroot-linux-gnu-g++
CFLAGS = -Wall -g -Iui -Iweb_server -Isystem -Ihal -I./ -fPIC -Ihal/include -Iengine
CXXFLAGS = $(CFLAGS) -std=c++14 -g -O0
CXXLIBS = -lpthread -lm -lrt -ldl -lseccomp
LDFLAGS = -Wl,--no-as-needed
#CC = gcc  # test in not target
#CXX = g++ # test in not target
# Source files and Object files
CSRC = main.c ui/gui.c ui/input.c web_server/web_server.c \
       system/system_server.c system/shared_memory.c system/dump_state.c \
       hal/hardware.c 
CXXSRC = engine/Allocator.cpp engine/Engine.cpp \
		 engine/LeftMotor.cpp engine/RightMotor.cpp engine/StateMachine.cpp \
		 engine/stdafx.cpp engine/xallocator.cpp
TOY_CXXSRC = hal/toy/camera_HAL_toy.cpp hal/toy/ControlThread.cpp \
             hal/utils/Thread.cpp hal/toy/Callbacks.cpp hal/toy/CallbackThread.cpp
OEM_CXXSRC = hal/oem/camera_HAL_oem.cpp hal/oem/ControlThread.cpp \

COBJ = $(CSRC:.c=.o)
CXXOBJ = $(CXXSRC:.cpp=.o)
OBJ = $(COBJ) $(CXXOBJ) engine/Fault.o
TOY_CXXOBJ = $(TOY_CXXSRC:.cpp=.o)
OEM_CXXOBJ = $(OEM_CXXSRC:.cpp=.o)

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

engine/Fault.o: engine/Fault.cpp
	$(CC) -g -c engine/Fault.cpp -o engine/Fault.o

# Rule to build libcamera.oem.so
libcamera.oem.so: $(OEM_CXXOBJ)
	$(CXX) -shared -fPIC -o $@ $^ $(CXXLIBS) $(LDFLAGS)

# Rule to build libcamera.toy.so
libcamera.toy.so: $(TOY_CXXOBJ)
	$(CXX) -shared -fPIC -o $@ $^ $(CXXLIBS) $(LDFLAGS)

# Clean target for removing compiled files
clean:
	rm -f $(COBJ) $(CXXOBJ) $(TOY_CXXOBJ) $(OEM_CXXOBJ) $(TARGET) libcamera.oem.so libcamera.toy.so\
		bin/filebrowser.db engine/Fault.o
	make -C drivers/simple_io clean
	make -C drivers/engine clean
	make -C drivers/state_machine clean
	make -C drivers/sensor clean
	rm -rf toy-fe/dist/*.js
	rm -rf toy-fe/dist/*.map
	rm -rf toy-be/frontend/dist/*
	rm -rf toy-be/toy-be
	rm -rf toy-fe/node_modules/*

# Phony targets
.PHONY: all clean libcamera.oem.so libcamera.toy.so

GO = GOGC=off go
GO_LDFLAGS += -s -w

.PHONY: toy-fe
toy-fe: | ; $(info $(M) building toy-fe…)
	cd toy-api && npm ci && cd ../toy-fe && npm ci && npm run build && cp -a dist/* ../toy-be/frontend/dist/

.PHONY: toy-be
toy-be: | ; $(info $(M) building toy be…)
	cd toy-be && env GOARCH=arm64 CGO_ENABLED=1 GOOS=linux CC=$(CC) CXX=$(CXX) GOGC=off GO111MODULE=on  $(GO) build -ldflags '$(GO_LDFLAGS)' -o toy-be

.PHONY: modules
modules:
	cd drivers/simple_io && make
	cd drivers/engine && make
	cd drivers/state_machine && make
	cd drivers/sensor && make

