#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "drivers/serial.h"  // for printk_serial

// stddef
typedef char* va_list;

extern void panic_assert(const char *file, uint32_t line, const char* message);

#define va_start(ap, last_arg) ((void)((ap) = (va_list)&(last_arg) + sizeof(last_arg)))
#define va_arg(ap, type) (*(type*)((ap) += sizeof(type), (ap) - sizeof(type)))
#define va_end(ap) ((void)(ap = (va_list)0))

// Boolean
#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1
// NULL
#define NULL ((void *)0)

#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#endif

