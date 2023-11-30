ASM = nasm
ASMFLAGS= -f elf32

CC = $(HOME)/toolchain/bin/i686-elf-gcc
LLIBS = -lgcc
CFLAGS = -g -ffreestanding -nostdlib -Wall -nostdinc

LD = $(HOME)/toolchain/bin/i686-elf-ld
LDFLAGS = -T linker.ld -nostdlib

OUTELF = XnixOS
BUILD_DIR = build
