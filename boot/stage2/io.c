/*
 *  XnixOS
 *
 *  boot/stage2/io.c
 */

#include <stdint.h>

// Write a byte out to the specified port.
void outb(uint16_t port, uint8_t value)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

void outw(uint16_t port, uint16_t value)
{
    __asm__ __volatile__ ( "outw %w0, %1" : : "a" (value), "id" (port) );
}

void outl (uint16_t port, uint32_t val)
{
     __asm__ volatile ("outl %0,%1":: "a" (val), "d" (port));
}

uint32_t inl(uint16_t port)
{
     static uint32_t ret;
     __asm__ volatile ("inl %1,%0":: "a" (ret), "dN" (port));
     return ret;
}
