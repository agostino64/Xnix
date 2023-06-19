global _start
extern kmain

MAGIC    equ 0x1badb002       ; Magic number - identifier for GRUB
FLAGS    equ 0x0
CHECKSUM equ -MAGIC    	      ; Checksum

section .text
align 4
  dd MAGIC       ; Magic number
  dd FLAGS       ; Flags
  dd CHECKSUM    ; Checksum

_start:
  jmp call_kernel  
  
call_kernel:
  mov esp, 0x20000   ; Place stack at 128KB boundary
  push eax           ; Push Multiboot Magic number onto the stack
  push ebx           ; Push address of Multiboot structure onto the stack
  call kmain         ; Call the main function of the C kernel
  ret

  cli                ; Halt the CPU if the kernel reaches this point
  hlt

  
