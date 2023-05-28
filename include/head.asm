section .text
align 4

global loader
extern kmain

FLAGS    equ 0
MAGIC    equ 0x1badb002       ; Magic number - identifier for GRUB
CHECKSUM equ -(MAGIC + FLAGS) ; Checksum

loader:
  mov esp, 0x200000  ; Place stack at 2MB boundary
  push eax           ; Push Multiboot Magic number onto the stack
  push ebx           ; Push address of Multiboot structure onto the stack
  call kmain         ; Call the main function of the C kernel

  cli                ; Halt the CPU if the kernel reaches this point
  hlt

section .multiboot
  dd MAGIC       ; Magic number
  dd FLAGS       ; Flags
  dd CHECKSUM    ; Checksum
