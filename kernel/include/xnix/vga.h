#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

void clear_vga();
void putc(char c, uint8_t color);
void puts(const char* str, uint8_t color);
void vprintf(const char* fmt, va_list args);
void printk(const char* fmt, ...);
void printk_buff(const char* msg, const void* buffer, uint32_t count);

#ifdef DEBUG
#define dprintk(...) printk(__VA_ARGS__)
#else
#define dprintk(...)
#endif

#endif
