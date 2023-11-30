RED := \033[0;31m
MGTA := \033[35m
NC := \033[0m
BOLD := \033[1m
CSV := \033[3m

ASM = nasm
ASMFLAGS= -f elf32

CC = $(HOME)/toolchain/bin/i686-elf-gcc
LLIBS = -lgcc
CFLAGS = -std=c11 -ffreestanding -nostdinc -fno-builtin -Wall -g

LD = $(HOME)/toolchain/bin/i686-elf-ld
LDFLAGS = -T link.ld -nostdlib

SRC_DIRS := boot drivers core init
BUILD_DIR := build
OUTELF := XnixOS
ISO_DIR := isodir
