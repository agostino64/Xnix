global reboot_hw 

section .text
reboot_hw:
    mov ah, 0           ; leer una tecla
    int 16h             ; interrupción de teclado
    jmp 0FFFFh:0        ; saltar al inicio del BIOS para reiniciar la PC
