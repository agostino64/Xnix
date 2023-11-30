/* XnixOS */
/* CPU specific functions for x86 */
#include <io.h>

unsigned char readCMOS(unsigned char addr)
{
   unsigned char ret;
   outb(0x70,addr);
   __asm__ __volatile__ ("jmp 1f; 1: jmp 1f;1:");
   ret = inb(0x71);
   __asm__ __volatile__ ("jmp 1f; 1: jmp 1f;1:");
   return ret;
}

void writeCMOS(unsigned char addr, unsigned int value)
{
   outb(0x70, addr);
   __asm__ __volatile__ ("jmp 1f; 1: jmp 1f;1:");
   outb(0x71, value);
   __asm__ __volatile__ ("jmp 1f; 1: jmp 1f;1:");
}

void reboot()
{
    outb(0x64, 0xFE); /* send reboot command */
}

void sti()
{
     __asm__ __volatile__ ("sti");
}

void cli()
{
     __asm__ __volatile__ ("cli");
}

void halt()
{
     __asm__ __volatile__ ("hlt");
}
