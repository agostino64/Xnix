global ksleep

section .text

ksleep:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]  ; Milisegundos

    ; Convertir milisegundos a microsegundos
    imul ecx, 1000

sleep_loop:
    ; Esperar 1 milisegundo
    mov al, 0x01
    out 0x80, al

    ; Decrementar contador de tiempo
    sub ecx, 1

    ; Comprobar si aún queda tiempo por dormir
    test ecx, ecx
    jnz sleep_loop

    mov esp, ebp
    pop ebp
    ret


