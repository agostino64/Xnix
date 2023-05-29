ASM = nasm
ASMFLAGS= -f elf

CC = i686-elf-gcc
LLIBS = -lgcc
CFLAGS = -g -ffreestanding -nostdlib -Wall -Werror -nostdinc

LD = i686-elf-ld
LDFLAGS = -T linker.ld -nostdlib

AR = i686-elf-ar
ARFLAGS= rcs

OUTELF = XnixOS
BUILD_DIR = build
