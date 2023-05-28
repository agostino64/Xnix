#pragma once
#include <stdint.h>

void kclear_screen();
void putc(char c, uint8_t color);
void kprint(const char* str);
void kprint_color(const char* str, uint8_t color);
