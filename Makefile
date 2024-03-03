# Makefile from JamesM's kernel tutorials and modified by Aarch64.

AS = nasm
CC = $(HOME)/toolchain/bin/i686-elf-gcc
LD = $(HOME)/toolchain/bin/i686-elf-ld

OUT = Image

CFLAGS += -std=c11 -nostdlib -nostdinc -fno-builtin  -I./include \
	 -fno-stack-protector -Wall -Wstrict-prototypes -g
LDFLAGS += -T linker.ld
ASFLAGS += -f elf

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
