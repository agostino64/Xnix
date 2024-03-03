#ifndef COMMON_H
#define COMMON_H

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.
typedef unsigned int   u32;
typedef          int   s32;
typedef unsigned short u16;
typedef          short s16;
typedef unsigned char  u8;
typedef          char  s8;

// stddef
typedef char* va_list;

#define va_start(ap, last_arg) ((void)((ap) = (va_list)&(last_arg) + sizeof(last_arg)))
#define va_arg(ap, type) (*(type*)((ap) += sizeof(type), (ap) - sizeof(type)))
#define va_end(ap) ((void)(ap = (va_list)0))

// stdbool
#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

// stddef
#define NULL ((void *)0)

#define KERN_ERR(format, ...) \
	printk("%s [%s] (%d): " format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

void outb(u16 port, u8 value);
void outw(u16 port, u16 value);
u8 inb(u16 port);
u16 inw(u16 port);

void memset(void *dest, u8 val, u32 len);
void memcpy(u8 *dest, const u8 *src, u32 len);
int strcmp(char *str1, char *str2);

#endif
