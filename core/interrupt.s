;
; interrupt.s -- Contains interrupt service routine wrappers.
;                Based on Bran's kernel development tutorials.
;
; Rewritten for JamesM's kernel development tutorials.

[EXTERN isr_handler]
[EXTERN irq_handler]

; This macro creates a stub for an ISR which does NOT pass its own
; error code (adds a dummy errcode byte).
%macro ISR_NOERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push byte 0
    push %1
    jmp isr_common_stub
%endmacro

; This macro creates a stub for an ISR which DOES pass its own error code.
%macro ISR_ERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push %1
    jmp isr_common_stub
%endmacro 

; This macro creates a stub for an IRQ.
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; --- Interrupt Service Routines ---
ISR_NOERRCODE  0
ISR_NOERRCODE  1
ISR_NOERRCODE  2
ISR_NOERRCODE  3
ISR_NOERRCODE  4
ISR_NOERRCODE  5
ISR_NOERRCODE  6
ISR_NOERRCODE  7
ISR_ERRCODE    8
ISR_NOERRCODE  9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; --- Interrupt Requests (Hardware) ---
IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

; --- Software Interrupt for yield() ---
ISR_NOERRCODE 128

; Common ISR Stub
isr_common_stub:
   pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
   mov ax, ds
   push eax                 ; Save the data segment descriptor
   mov ax, 0x10             ; Load the kernel data segment descriptor
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   push esp                 ; Pass registers_t* to the C handler
   call isr_handler
   add esp, 4               ; Clean up the stack

   pop eax                  ; Reload the original data segment descriptor
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   popa                     ; Pops edi,esi,ebp...
   add esp, 8               ; Cleans up the error code and pushed ISR number
   sti
   iret                     ; pops CS, EIP, EFLAGS, SS, and ESP

; Common IRQ Stub
irq_common_stub:
   pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
   mov ax, ds
   push eax                 ; Save the data segment descriptor
   mov ax, 0x10             ; Load the kernel data segment descriptor
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   push esp                 ; Pass registers_t* to the C handler
   call irq_handler
   add esp, 4               ; Clean up the stack

   pop eax                  ; Reload the original data segment descriptor (Corrected from pop ebx)
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   popa                     ; Pops edi,esi,ebp...
   add esp, 8               ; Cleans up the pushed error code and pushed ISR number
   sti
   iret                     ; pops CS, EIP, EFLAGS, SS, and ESP
