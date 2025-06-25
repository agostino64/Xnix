// heap.h -- Interface for kernel heap functions, also provides
//            a placement malloc() for use before the heap is 
//            initialised.
//            Written for JamesM's kernel development tutorials.

#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include "common.h"
#include "paging.h"
#include "ordered_array.h"

#define KHEAP_START         0x50000000
#define KHEAP_INITIAL_SIZE  0x100000
#define KHEAP_MAX_ADDRESS 0xFFFF000

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0xDEADBEEF
#define HEAP_MIN_SIZE     0x70000

/**
   Size information for a hole/block
**/
typedef struct
{
    uint32_t magic;   // Magic number, used for error checking and identification.
    uint8_t is_hole;   // 1 if this is a hole. 0 if this is a block.
    uint32_t size;    // size of the block, including the end footer.
} header_t;

typedef struct
{
    uint32_t magic;     // Magic number, same as in header_t.
    header_t *header; // Pointer to the block header.
} footer_t;

typedef struct
{
    ordered_array_t index;
    uint32_t start_address; // The start of our allocated space.
    uint32_t end_address;   // The end of our allocated space. May be expanded up to max_address.
    uint32_t max_address;   // The maximum address the heap can be expanded to.
    uint8_t supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
    uint8_t readonly;       // Should extra pages requested by us be mapped as read-only?
} heap_t;

/**
   Create a new heap.
**/
heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly);

/**
   Allocates a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting
   on a page boundary.
**/
void *alloc(uint32_t size, uint8_t page_align, heap_t *heap);

//With physical address
void alloc_frame_p(page_t *page, int is_kernel, int is_writeable, uint32_t phys);

/**
   Releases a block allocated with 'alloc'.
**/
void free(void *p, heap_t *heap);

/**
   Allocate a chunk of memory, sz in size. If align == 1,
   the chunk must be page-aligned. If phys != 0, the physical
   location of the allocated chunk will be stored into phys.

   This is the internal version of kmalloc. More user-friendly
   parameter representations are available in kmalloc, kmalloc_a,
   kmalloc_ap, kmalloc_p.
**/
uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys);

/**
   Allocate a chunk of memory, sz in size. The chunk must be
   page aligned.
**/
uint32_t kmalloc_a(uint32_t sz);

/**
   Allocate a chunk of memory, sz in size. The physical address
   is returned in phys. Phys MUST be a valid pointer to uint32_t!
**/
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys);

/**
   Allocate a chunk of memory, sz in size. The physical address 
   is returned in phys. It must be page-aligned.
**/
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

/**
   General allocation function.
**/
uint32_t kmalloc(uint32_t sz);

/**
   General deallocation function.
**/
void kfree(void *p);

/**
   General reallocation function.
**/
void* krealloc(void* ptr, uint32_t old_size, uint32_t new_size);

/**
   Return amount of memory used
**/
uint32_t get_memory_usage(void);

/**
   Expand and contract heap
**/
void expand(uint32_t new_size, heap_t *heap);
uint32_t contract(uint32_t new_size, heap_t *heap);

#endif // HEAP_H
