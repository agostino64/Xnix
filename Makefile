include toolchain.mk

SOURCES_ASM := $(wildcard $(addsuffix /*.asm, $(SRC_DIRS)))
OBJECTS_ASM := $(patsubst %.asm, %.asm.o, $(SOURCES_ASM))

SOURCES_C := $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJECTS_C := $(patsubst %.c, %.c.o, $(SOURCES_C))

all: $(OBJECTS_ASM) $(OBJECTS_C) link

%.asm.o: %.asm
	@mkdir -p $(BUILD_DIR)
	@$(ASM) $(ASMFLAGS) -o $@ $<
	@mv $@ $(notdir $@/build)
	@echo -e "${MGTA}--> Compiled:${NC} ${CSV}" $< "${NC}"

%.c.o: %.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -I./include -c $< -o $@
	@mv $@ $(notdir $@/build)
	@echo -e "${MGTA}--> Compiled:${NC} ${CSV}" $< "${NC}"

link:
	@mkdir -p $(BUILD_DIR)
	@$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(OUTELF) $(addprefix build/, *.o)
	@echo -e "\n${MGTA}--> Created:${NC} ${BOLD}XnixOS" "${NC}"

run:
	@qemu-system-i386 -kernel $(BUILD_DIR)/$(OUTELF)

clean:
	@rm -rf build

mkiso:
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp -r boot/grub.cfg $(ISO_DIR)/boot/grub/
	@cp -r $(BUILD_DIR)/$(OUTELF) $(ISO_DIR)/boot/
	@grub2-mkrescue -o $(OUTELF).iso $(ISO_DIR)
