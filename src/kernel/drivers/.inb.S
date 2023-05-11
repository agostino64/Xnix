.globl inb

.section .text

inb:
    # Guardar el valor del registro ebp en la pila
    pushl %ebp

    # Establecer ebp como el valor actual del puntero de la pila (esp)
    movl %esp, %ebp

    # Mover el valor del primer parámetro (puerto) a dx
    movw 8(%ebp), %dx

    # Leer un byte desde el puerto en el registro al
    inb (%dx), %al

    # Mover el valor de al (el byte leído) a la posición de memoria del segundo parámetro
    movb %al, 12(%ebp)

    # Restaurar el valor de ebp desde la pila
    popl %ebp

    # Retornar a la dirección de retorno guardada
    ret
