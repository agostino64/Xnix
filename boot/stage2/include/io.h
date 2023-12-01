#ifndef IO_H
#define IO_H

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t value);
void outl (uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
#define out8_p(value,port) outb_p(port,value)
#define in8_p(port) inb_p(port)

#endif
