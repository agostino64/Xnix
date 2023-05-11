.global i686_outb
i686_outb:
.intel_syntax noprefix
mov dx, [esp + 4]
mov al, [esp + 8]
out dx, al
ret

.global i686_inb
i686_inb:
.intel_syntax noprefix
mov dx, [esp + 4]
xor eax, eax
in al, dx
ret

.global i686_Panic
i686_Panic:
.intel_syntax noprefix
cli
hlt

.global i686_EnableInterrupts
i686_EnableInterrupts:
.intel_syntax noprefix
sti
ret

.global i686_DisableInterrupts
i686_DisableInterrupts:
.intel_syntax noprefix
cli
ret

.global crash_me
crash_me:
.intel_syntax noprefix
# div by 0
# mov ecx, 0x1337
# mov eax, 0
# div eax
int 0x80
ret
