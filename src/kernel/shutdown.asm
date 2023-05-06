global shutdown_hw 

section .text
shutdown_hw:
    cli                 ; deshabilitar interrupciones
    xor ax, ax          ; ax = 0
    mov es, ax          ; es = 0
    mov bx, ax          ; bx = 0
    mov dx, 0x8         ; dx = 0x8 (función de apagado)
    int 0x15            ; llamada a la BIOS para apagar la PC
    hlt                 ; si la llamada falla, detener la ejecución del programa
