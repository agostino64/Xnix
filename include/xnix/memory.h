#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

void* memcpy(void* dst, const void* src, u16 num);
void* memset(void* ptr, int value, u16 num);
int memcmp(const void* ptr1, const void* ptr2, u16 num);

#endif