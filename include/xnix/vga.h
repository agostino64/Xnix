// vga.h -- Defines the interface for vga.c
// From JamesM's kernel development tutorials.

#ifndef VGA_H
#define VGA_H

#include "common.h"

// Write a single character out to the screen.
void put(const char c);

// Clear the screen to all black.
void clear_screen(void);

// Output a null-terminated ASCII string to the monitor.
void write(const char *c);

// Print with variable handling (still differs from printk)
void vprintk(const char* fmt, va_list args);

// Print with suppport for arguments
void printk(const char* fmt, ...);

#endif
