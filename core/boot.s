;
; boot.s -- Kernel start location. Also defines multiboot header.
; Based on Bran's kernel development tutorial file start.asm
;

MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

mboot:
  dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file
  dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
  dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct
   
  dd  mboot                     ; Location of this descriptor
  dd  code                      ; Start of kernel '.text' (code) section.
  dd  bss                       ; End of kernel '.data' section.
  dd  end                       ; End of kernel.
  dd  _start                    ; Kernel entry point (initial EIP).

[GLOBAL _start]                  ; Kernel entry point.
[EXTERN start_kernel]           ; This is the entry point of our C code

_start:    
    cli                        ; disable interrupts early
    
    ;─── align stack to 16 bytes ─────────────────────────────────────────────
    mov    eax, esp
    and    eax, 0xFFFFFFF0
    mov    esp, eax
    
    ;─── pass multiboot parameters ───────────────────────────────────────────
    push ebx      ; Pushes mboot_ptr (the right-most argument)
    push esp      ; Pushes initial_stack (the left-most argument)

    push   ret_addr
    jmp    start_kernel        ; C: start_kernel(mboot_ptr, initial_stack)
ret_addr:
    ;       |                  |
    ;       +------------------+
    ; esp → |   ret_addr       |  (Return address for start_kernel)
    ;      +------------------+
    ; esp+4 | initial_stack    |  (Argument 1)
    ;       +------------------+
    ; esp+8 | mboot_ptr        |  (Argument 2)
    ;       +------------------+
    ;       | ...              |
    ; 
    ; Clean up the 2 arguments (4 bytes each) pushed to the stack
    add esp, 8
    
    ; Now the stack is restored to its state before the pushes.
    ; Execution can continue, or more likely, halt.
    hlt