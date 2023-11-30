;*
;*  xnix/boot/boot.asm
;*
;*  Copyright (C) 2023  Agustin Gutierrez
;*

; These are the multiboot macroes, they are used to configure how GRUB
; loads the kernel
MBOOT_PAGE_ALIGN    equ 1<<0 ; Loads everything on page boundaries
MBOOT_MEM_INFO      equ 1<<1 ; Load memory map
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Required (MAGIC)

MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


bits 32

global mboot
extern code ; .text
extern bss  ; .bss
extern end  ; defined in linker script

mboot:
  dd  MBOOT_HEADER_MAGIC
  dd  MBOOT_HEADER_FLAGS
  dd  MBOOT_CHECKSUM
   
  dd  mboot
  dd  code
  dd  bss
  dd  end
  dd  start ; Initial EIP

global start ; Entry Point
extern start_kernel

start:
  push    esp ; Stack location (GRUB doesn't tell us)
  push    ebx ; Multiboot header location

  ; Execute the kernel:
  cli
  call start_kernel
  jmp $
