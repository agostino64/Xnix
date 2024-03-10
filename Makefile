# Makefile from JamesM's kernel tutorials and modified by Aarch64.

AS = nasm
CC = gcc
LD = ld

OUT = Image

CFLAGS += -std=c11 -nostdlib -nostdinc -fno-builtin  -I./include \
	 -fno-stack-protector -Wall -Wstrict-prototypes -m32 -g
LDFLAGS += -T linker.ld -m elf_i386
ASFLAGS += -f elf -F dwarf -g

SOURCES = core/boot.o \
	  core/interrupt.o \
	  core/cpu_asm.o \
	  core/gdt.o \
	  core/main.o \
	  core/descriptor_tables.o \
	  core/common.o \
	  core/timer.o \
	  core/cpu.o \
	  core/isr.o \
	  drivers/vga.o \
	  drivers/keyb.o \
	  drivers/shell.o

all: $(SOURCES) link

link:
	@$(LD) $(LDFLAGS) -o $(OUT) $(SOURCES)
	@echo ' '
	@echo $(OUT)
	
	
clean:
	rm core/*.o drivers/*.o $(OUT)
