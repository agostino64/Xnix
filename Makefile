# Makefile from JamesM's kernel tutorials and modified by Aarch64.

DEBUG = 1

AS = nasm
CC = gcc
LD = ld

OUT = Image

CFLAGS += -std=c11 -nostdlib -nostdinc -fno-builtin  -I./include \
	 -fno-stack-protector -m32 -march=i386
LDFLAGS += -T linker.ld -m elf_i386
ASFLAGS += -f elf -F dwarf

ifeq ($(DEBUG),1)
    CFLAGS  += -DDEBUG -g -Wall -Wstrict-prototypes
    ASFLAGS += -g
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
	  drivers/keyb.o

all: $(SOURCES) link

link:
	@$(LD) $(LDFLAGS) -o $(OUT) $(SOURCES)
	@echo ' '
	@echo $(OUT)
	
	
clean:
	rm core/*.o drivers/*.o $(OUT)
