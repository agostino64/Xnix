#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

void kclear_screen();
void putc(char c, uint8_t color);
void puts(const char* str, uint8_t color);
void vprintf(const char* fmt, va_list args);
void printk(const char* fmt, ...);
void printk_buff(const char* msg, const void* buffer, uint32_t count);
