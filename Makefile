# Makefile for compiling kernel module

# Name of the kernel module
MODULE_NAME = demo_module
MY_OUTPUT_DIR = output

ccflags-y := \
  -DDEBUG \
  -ggdb3 \
  -std=gnu99 \
  -Werror \
  -Wframe-larger-than=1000000000 \
  -Wno-declaration-after-statement \
  $(CCFLAGS)
  
# Source file(s) of the kernel module
SRC = ./km/$(MODULE_NAME).c

# Kernel source directory
KERNEL_SRC_DIR ?= /lib/modules/$(shell uname -r)/build

# Output directory
OUTPUT_DIR ?= .

# Flags for the kernel module
EXTRA_CFLAGS += -Werror

# Target
obj-m += $(MODULE_NAME).o

# Rule to build the module
$(MODULE_NAME)-objs := $(SRC:.c=.o)

all:
	make -C $(KERNEL_SRC_DIR) M=$(PWD) modules
clean:
	make -C $(KERNEL_SRC_DIR) M=$(PWD) clean
.PHONY: all clean
