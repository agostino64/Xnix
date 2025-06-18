// common.c -- Defines some global functions.
// From JamesM's kernel development tutorials.

#include <xnix/common.h>

// Write a byte out to the specified port.
void outb(u16 port, u8 value)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(u16 port, u16 value)
{
    __asm__ __volatile__ ( "outw %w0, %1" : : "a" (value), "id" (port) );
}

u8 inb(u16 port)
{
   u8 ret;
   __asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

u16 inw(u16 port)
{
   u16 ret;
   __asm__ __volatile__ ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

