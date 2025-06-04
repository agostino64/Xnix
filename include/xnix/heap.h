#ifndef HEAP_H
#define HEAP_H

#include "common.h"

#define HEAP_PHYS_ADDR  0x100000       // Physical start of the heap (1MB)
#define HEAP_VIRT_ADDR  0xC0000000     // Virtual start of the heap
#define HEAP_SIZE       0x100000       // 1MB heap size

typedef struct heap_header {
    u32 size;
    u8 is_free;
    struct heap_header* next;
} heap_header_t;

void init_heap(void);
void* kmalloc(u32 size);
void kfree(void* ptr);
void* krealloc(void* ptr, u32 old_size, u32 new_size);

#endif

