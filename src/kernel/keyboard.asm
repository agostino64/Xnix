global inb

section .text

inb:
    push ebp
    mov ebp, esp
    mov dx, word [ebp+8] 	; move port number to DX
    in al, dx            	; read byte from port into AL
    mov byte [ebp+12], al 	; move byte value to return value
    pop ebp
    ret

