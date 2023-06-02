include toolchain.mk

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
	@$(CC) $(CFLAGS) -I include/ -L build/ -lxnix -c $< -o $@
	@echo "--> Compiled: " $<

link:
	@make -C ./lib
	@$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(OUTELF) $(OBJECTS_ASM) $(OBJECTS_C) -L build/ -lxnix
	@echo "--> Created:  XnixOS"
	
run:
	@qemu-system-i386 --kernel $(BUILD_DIR)/$(OUTELF)

clean:
	@rm -rf $(BUILD_DIR)

mkiso:
	@mkdir -p isodir/boot/grub
	@cp -r grub.cfg isodir/boot/grub/
	@cp -r build/XnixOS isodir/boot/
	@grub2-mkrescue -o XnixOS.iso isodir
