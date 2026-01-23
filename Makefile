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

DEBUG      	:= 1
OUT        	:= Image
FSPATH     	:= fs_files/
BUILD_INFO 	:= build_info.h
BUILD_NUM_FILE  := .build_number

TARGET_CFLAGS += -ffreestanding -mno-red-zone \
	-I./include -fno-stack-protector  \
	-mno-sse -mno-sse2 -mno-mmx -msoft-float
TARGET_LDFLAGS += -T linker.ld -nostdlib
TARGET_LINKFLAGS += -f elf
TARGET_LIBS += -lgcc

# Use 'find' to get all the files and folders (recursive) in the directory
# Use 'sed' to remove FSPATH from returned paths (and make them relative)
FILES = $(shell find $(FSPATH) -mindepth 1 | sed 's|^$(FSPATH)||')

ifeq ($(DEBUG),1)
    TARGET_CFLAGS  += -DDEBUG -DLOG_LEVEL=LOG_LEVEL_DEBUG \
    	-g -Wall -O0 -Wextra -Wstrict-prototypes -fno-omit-frame-pointer
    TARGET_LINKFLAGS += -F dwarf -g
endif

ifeq ($(DEBUG),0)
    TARGET_CFLAGS  += -O -fomit-frame-pointer
endif

ifeq ($(CC),clang)
    TARGET_CFLAGS +=
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
	  core/memory.o \
      	  core/string.o \
	  core/switch_task.o \
	  core/version.o \
	  drivers/timer.o \
	  drivers/keyb.o \
	  drivers/serial.o

include build_scripts/config.mk

.PHONY: $(BUILD_INFO) FORCE

all: Image initrd

include build_scripts/toolchain.mk

FORCE:

$(BUILD_INFO): FORCE
	@echo "Generating system info header"
	@if [ ! -f $(BUILD_NUM_FILE) ]; then \
		echo 0 > $(BUILD_NUM_FILE); \
	fi
	@BUILD_NUMBER=$$(($$(cat $(BUILD_NUM_FILE)) + 1)); \
	echo $$BUILD_NUMBER > $(BUILD_NUM_FILE); \
	{ \
	printf "#ifndef BUILD_INFO_H\n#define BUILD_INFO_H\n"; \
	printf "#define BUILD_USER \"%s\"\n" "$(shell whoami)"; \
	printf "#define BUILD_OS   \"%s\"\n" "$(shell uname -s)"; \
	printf "#define BUILD_NUM  \"%d\"\n" $$BUILD_NUMBER; \
	printf "#endif /* BUILD_INFO_H */\n"; \
	} > $@.tmp; \
	cmp -s $@.tmp $@ || mv $@.tmp $@; \
	rm -f $@.tmp

Image: $(SOURCES)
	@echo "  LD    $(OUT)"
	@$(TARGET_LD) $(TARGET_LDFLAGS) -o $(OUT) $(SOURCES) $(TARGET_LIBS)
	
core/version.o: core/version.c FORCE
	@$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@
	
%.o: %.c | $(BUILD_INFO)
	@echo "  CC    $<"
	@$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

%.o: %.s
	@echo "  AS    $<"
	@$(TARGET_ASM) $(TARGET_LINKFLAGS) $< -o $@

clean:
	rm -f core/*.o drivers/*.o *.o *.img $(OUT) generate_initrd.o initrd.img $(BUILD_INFO) $(BUILD_NUM_FILE)
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

run: Image initrd.img
	qemu-system-i386 -kernel Image -initrd initrd.img -serial stdio
