# Makefile from JamesM's kernel tutorials and modified by Agustin Gutierrez.
#
# How to run iso...
# qemu-system-i386 -cdrom xnix_i386_xxxxxxxxxx.iso -serial stdio
#
# How to run kernel image...
# qemu-system-i386 -kernel Image -serial stdio -initrd initrd.img
#
# How to dump kernel image
# objdump -D -Mintel -b elf32-i386 -m i386 Image | less > dump.txt
#

DEBUG = 1

AS = nasm
CC = i686-elf/bin/i686-elf-gcc
LD = i686-elf/bin/i686-elf-ld

OUT = Image

FSPATH = fs_files/
# Use 'find' to get all the files and folders (recursive) in the directory
# Use 'sed' to remove FSPATH from returned paths (and make them relative)
FILES = $(shell find $(FSPATH) -mindepth 1 | sed 's|^$(FSPATH)||')

CFLAGS += -std=c11 -nostdlib -nostdinc -fno-builtin -fstrength-reduce \
 -fomit-frame-pointer -I./include -fno-stack-protector
LDFLAGS += -T linker.ld
ASFLAGS += -f elf

ifeq ($(DEBUG),1)
    CFLAGS  += -DDEBUG -DLOG_LEVEL=LOG_LEVEL_DEBUG -g -Wall -Wstrict-prototypes
    ASFLAGS += -F dwarf -g
endif

ifeq ($(DEBUG),0)
    CFLAGS  += -O
endif

ifeq ($(CC),clang)
    CFLAGS += -mno-sse -mno-mmx -msoft-float
endif

SOURCES = core/boot.o \
	  core/interrupt.o \
	  core/cpu_asm.o \
	  core/process.o \
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
	  core/initrd.o \
	  core/fs.o \
	  core/ordered_array.o \
	  core/rtc.o \
	  core/panic.o \
	  core/task.o \
	  core/switch_task.o \
	  drivers/timer.o \
	  drivers/keyb.o \
	  drivers/serial.o

all: Image initrd

Image: $(SOURCES)
	@$(LD) $(LDFLAGS) -o $(OUT) $(SOURCES)
	@echo ' '
	@echo $(OUT)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f core/*.o drivers/*.o *.o *.img $(OUT) generate_initrd.o initrd.img
	rm -rf mkiso
	
iso: Image
	mkdir -p mkiso/boot/grub
	cp initrd.img mkiso/boot
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

initrd: generate_initrd.c Image
	@echo "Generating initialRamDisk ..."
	gcc -Wall generate_initrd.c -o generate_initrd.o
	./generate_initrd.o $(FSPATH) $(FILES)
