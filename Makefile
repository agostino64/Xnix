# Makefile from JamesM's kernel tutorials and modified by Aarch64.
#
# How to run iso...
# qemu-system-i386 -cdrom xnix_i386_xxxxxxxxxx.iso -serial stdio
#
# How to run kernel image...
# qemu-system-i386 -kernel Image -serial stdio
#

DEBUG = 1

AS = nasm
CC = gcc
LD = ld

OUT = Image

CFLAGS += -std=c11 -nostdlib -nostdinc -fno-builtin  -I./include \
	 -fno-stack-protector -m32 -march=i386
LDFLAGS += -T linker.ld -m elf_i386
ASFLAGS += -f elf

ifeq ($(DEBUG),1)
    CFLAGS  += -DLOG_LEVEL=LOG_LEVEL_DEBUG -g -Wall -Wstrict-prototypes
    ASFLAGS += -F dwarf -g
endif

SOURCES = core/boot.o \
	  core/interrupt.o \
	  core/cpu_asm.o \
	  core/gdt.o \
	  core/main.o \
	  core/descriptor_tables.o \
	  core/common.o \
	  core/cpu.o \
	  core/isr.o \
	  core/heap.o \
	  core/paging.o \
	  core/vga.o \
	  core/shell.o \
	  core/drvmanager.o \
	  drivers/timer.o \
	  drivers/keyb.o \
	  drivers/serial.o

all: $(SOURCES) link

link:
	@$(LD) $(LDFLAGS) -o $(OUT) $(SOURCES)
	@echo ' '
	@echo $(OUT)
	
	
clean:
	rm core/*.o drivers/*.o $(OUT) && rm -r mkiso
	
iso: Image
	mkdir -p mkiso/boot/grub
	cp Image mkiso/boot/Image
	cp grub.cfg mkiso/boot/grub/grub.cfg
	@echo ">> Generating ISO..."
	@if command -v grub2-mkrescue >/dev/null 2>&1; then \
		grub2-mkrescue mkiso -o xnix_i386_$(shell date +%s).iso; \
	elif command -v grub-mkrescue >/dev/null 2>&1; then \
		grub-mkrescue mkiso -o xnix_i386_$(shell date +%s).iso; \
	else \
		echo "ERROR: Neither grub2-mkrescue nor grub-mkrescue found."; \
		exit 1; \
	fi

