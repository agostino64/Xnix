section .text
[GLOBAL switchTask]
switchTask:
    pusha
    pushf

    mov eax, cr3         ; Push CR3
    push eax

    mov eax, [esp + 44]  ; The first argument, where to save

    mov [eax + 4], ebx   ; EBX
    mov [eax + 8], ecx   ; ECX
    mov [eax + 12], edx  ; EDX
    mov [eax + 16], esi  ; ESI
    mov [eax + 20], edi  ; EDI

    mov ebx, [esp + 36]  ; EAX
    mov ecx, [esp + 40]  ; EIP
    mov edx, [esp + 20]  ; ESP
    add edx, 4           ; Remove the return address

    mov esi, [esp + 16]  ; EBP
    mov edi, [esp + 4]   ; EFLAGS

    mov [eax + 0], ebx   ; Save EAX
    mov [eax + 24], edx  ; Save ESP
    mov [eax + 28], esi  ; Save EBP
    mov [eax + 32], ecx  ; Save EIP
    mov [eax + 36], edi  ; Save EFLAGS

    pop ebx              ; Restore CR3
    mov [eax + 40], ebx
    push ebx             ; Push CR3 again

    mov eax, [esp + 48]  ; Load new task struct

    mov ebx, [eax + 4]
    mov ecx, [eax + 8]
    mov edx, [eax + 12]
    mov esi, [eax + 16]
    mov edi, [eax + 20]
    mov ebp, [eax + 28]

    push eax
    mov eax, [eax + 36]  ; EFLAGS
    push eax
    popf
    pop eax

    mov esp, [eax + 24]  ; ESP
    push eax
    mov eax, [eax + 40]  ; CR3
    mov cr3, eax
    pop eax

    push eax
    mov eax, [eax + 32]  ; EIP
    xchg eax, [esp]      ; Swap return addr with EIP
    mov eax, [eax]       ; EAX
    ret                  ; Jump to new task!
