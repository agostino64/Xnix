COMPILE_PATH = ../.toolchains/i686-elf/bin/

ASM = nasm
ASMFLAGS= -f elf

CC = $(COMPILE_PATH)i686-elf-gcc
LLIBS = -lgcc
CFLAGS = -g -ffreestanding -nostdlib -Wall -Werror -nostdinc -I include/

LD = $(COMPILE_PATH)i686-elf-ld
LDFLAGS = -T linker.ld -nostdlib

OUTELF = XnixOS
BUILD_DIR = build

SOURCES_ASM := $(wildcard *.asm)
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIR)/%.asm.o, $(SOURCES_ASM))

SOURCES_C := $(wildcard *.c)
OBJECTS_C := $(patsubst %.c, $(BUILD_DIR)/%.c.o, $(SOURCES_C))

all: $(OBJECTS_ASM) $(OBJECTS_C) link

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	@$(ASM) $(ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/%.c.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<
	
link:
	@$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(OUTELF) $(OBJECTS_ASM) $(OBJECTS_C)
	@echo "--> Created:  XnixOS"
	
run:
	@qemu-system-i386 --kernel $(BUILD_DIR)/$(OUTELF)

clean:
	@rm -rf $(BUILD_DIR)

