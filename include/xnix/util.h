#ifndef UTIL_H
#define UTIL_H

#include <xnix/isr.h>
#include <xnix/vga.h>
#include <xnix/io.h>
#include <stdint.h>

//#define DEBUG

void panic(registers_t* regs);
void panic_assert(const char *file, uint32_t line, const char *desc);

void beep();

#endif
