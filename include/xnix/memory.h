#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "common.h"

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);

#endif
