#ifndef COMMON_H
#define COMMON_H

#include "drivers/serial.h"  // for printk_serial

// Tipos estándar para 32-bit
typedef unsigned int   u32;
typedef          int   s32;
typedef unsigned short u16;
typedef          short s16;
typedef unsigned char  u8;
typedef          char  s8;

// stddef
typedef char* va_list;

extern void panic_assert(const char *file, u32 line, const char* message);

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

void outb(u16 port, u8 value);
void outw(u16 port, u16 value);
u8 inb(u16 port);
u16 inw(u16 port);

#endif

