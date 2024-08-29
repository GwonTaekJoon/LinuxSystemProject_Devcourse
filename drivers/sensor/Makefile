obj-m := sensor.o

export BUILDROOT_DIR = /home/gwontaekjoon/Desktop/buildroot
export ARCH=arm64
export CROSS_COMPILE = $(BUILDROOT_DIR)/output/host/bin/aarch64-buildroot-linux-gnu-

KERNELDIR ?= $(BUILDROOT_DIR)/output/build/linux-custom

all default: modules
install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
