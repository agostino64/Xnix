include toolchain.mk

SOURCES_ASM := $(wildcard boot/*.asm kernel/*.asm drivers/*.asm cpu/*.asm)
OBJECTS_ASM := $(patsubst %.asm, %.asm.o, $(SOURCES_ASM))

SOURCES_C := $(wildcard boot/*.c kernel/*.c drivers/*.c cpu/*.c)
OBJECTS_C := $(patsubst %.c, %.c.o, $(SOURCES_C))

all: $(OBJECTS_ASM) $(OBJECTS_C) link

%.asm.o: %.asm
	@$(ASM) $(ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

%.c.o: %.c
	@$(CC) $(CFLAGS) -I include/ -c $< -o $@
	@echo "--> Compiled: " $<

link:
	@mkdir -p $(BUILD_DIR)
	@$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(OUTELF) $(OBJECTS_ASM) $(OBJECTS_C)
	@echo "--> Created:  XnixOS"

run:
	@qemu-system-i386 -kernel build/XnixOS

clean:
	@rm -rf build boot/*.o drivers/*.o cpu/*.o kernel/*.o

mkiso:
	@mkdir -p isodir/boot/grub
	@cp -r grub.cfg isodir/boot/grub/
	@cp -r build/XnixOS isodir/boot/
	@grub2-mkrescue -o XnixOS.iso isodir
