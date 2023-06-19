ASM = nasm
ASMFLAGS= -f elf32

CC = ../fedora-GCC-i686/bin/i686-elf-gcc
LLIBS = -lgcc
CFLAGS = -g -ffreestanding -nostdlib -Wall -nostdinc

LD = ../fedora-GCC-i686/bin/i686-elf-ld
LDFLAGS = -T linker.ld -nostdlib

OUTELF = XnixOS
BUILD_DIR = build
