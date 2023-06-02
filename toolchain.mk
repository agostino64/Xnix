ASM = nasm
ASMFLAGS= -f elf

CC = ../.toolchains/i686-elf/bin/i686-elf-gcc
LLIBS = -lgcc
CFLAGS = -g -ffreestanding -nostdlib -Wall -nostdinc

LD = ../.toolchains/i686-elf/bin/i686-elf-ld
LDFLAGS = -T linker.ld -nostdlib

AR = ../.toolchains/i686-elf/bin/i686-elf-ar
ARFLAGS= rcs

OUTELF = XnixOS
BUILD_DIR = build
