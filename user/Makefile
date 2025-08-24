# User services Makefile

ARCH ?= x86_64
DEBUG ?= 0

# Directories
SERVICES_DIR := services
OUT_DIR := ../../out/user

# Tools
CROSS_COMPILE ?= $(ARCH)-elf-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

# Flags
CFLAGS := -Wall -Wextra -ffreestanding -O2
ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
endif

# Services
SERVICES := init devd vfsd netd

.PHONY: all clean $(SERVICES)

all: $(SERVICES)

$(SERVICES): %:
	@mkdir -p $(OUT_DIR)/$@
	$(CC) $(CFLAGS) -c $(SERVICES_DIR)/$@/main.c -o $(OUT_DIR)/$@/main.o
	$(LD) -o $(OUT_DIR)/$@/service $(OUT_DIR)/$@/main.o

clean:
	rm -rf $(OUT_DIR)
