/*
 *  Xnix
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  Simple kernel heap allocator for Xnix operating system.
 *  Provides basic dynamic memory management functions: kmalloc, kfree, krealloc.
 *  Works on a single, contiguous memory region mapped by the kernel.
 */

#include <xnix/heap.h>
#include <xnix/vga.h>
#include <xnix/common.h>
#include <xnix/log.h>

// Heap starts at a high virtual address mapped by paging.
// HEAP_START is defined externally via paging.
#define HEAP_START HEAP_VIRT_ADDR
#define HEAP_INITIAL_SIZE HEAP_SIZE

// Global heap pointers
static heap_header_t* heap_start = (heap_header_t*)HEAP_START;  // First heap block
static heap_header_t* free_list = NULL;                         // Pointer to free list

/**
 * Initializes the heap by creating a single large free block.
 * Should be called once at kernel startup.
 */
void init_heap(void) {
    heap_start->size = HEAP_INITIAL_SIZE - sizeof(heap_header_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    free_list = heap_start;

    KLOG(LOG_LEVEL_DEBUG, "[heap] Initialized at 0x%X with size %u bytes\n", (u32)heap_start, heap_start->size);
}

/**
 * Splits a block if it's significantly larger than requested size,
 * and marks the allocated block as used.
 *
 * @param block Pointer to the heap block to allocate from.
 * @param size  Number of bytes requested by the caller.
 * @return Pointer to usable memory (after the block header).
 */
static void* allocate_block(heap_header_t* block, u32 size) {
    if (block->size > size + sizeof(heap_header_t)) {
        // Split the block and create a new free block
        heap_header_t* new_block = (heap_header_t*)((u8*)block + sizeof(heap_header_t) + size);
        new_block->size = block->size - size - sizeof(heap_header_t);
        new_block->is_free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;

        KLOG(LOG_LEVEL_DEBUG, "[heap] Block split at 0x%X\n", (u32)new_block);
    }

    block->is_free = 0;
    return (void*)((u8*)block + sizeof(heap_header_t));
}

/**
 * Allocates memory from the kernel heap.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory or NULL on failure.
 */
void* kmalloc(u32 size) {
    heap_header_t* current = free_list;
    while (current) {
        if (current->is_free && current->size >= size) {
            void* ptr = allocate_block(current, size);
            KLOG(LOG_LEVEL_DEBUG, "[heap] Allocated %u bytes at 0x%X\n", size, (u32)ptr);
            return ptr;
        }
        current = current->next;
    }

    KLOG(LOG_LEVEL_ERROR, "[heap] ERROR: Out of memory\n");
    return NULL;
}

/**
 * Allocates memory and zeroes it.
 *
 * @param size Number of bytes to allocate.
 * @return Zeroed memory pointer or NULL on failure.
 */
void* kmalloc_zero(u32 size) {
    void* ptr = kmalloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

/**
 * Allocates memory with alignment.
 *
 * @param size  Number of bytes to allocate.
 * @param align Alignment in bytes (must be power of two).
 * @return Pointer to aligned memory or NULL on failure.
 */
void* kmalloc_aligned(u32 size, u32 align) {
    u32 mask = align - 1;
    heap_header_t* current = free_list;

    while (current) {
        if (current->is_free && current->size >= size) {
            u32 block_addr = (u32)current + sizeof(heap_header_t);
            u32 aligned_addr = (block_addr + mask) & ~mask;
            u32 offset = aligned_addr - block_addr;

            if (current->size >= size + offset) {
                if (offset > 0) {
                    // Split leading misaligned space
                    heap_header_t* aligned_block = (heap_header_t*)((u8*)current + offset);
                    aligned_block->size = current->size - offset;
                    aligned_block->is_free = 1;
                    aligned_block->next = current->next;

                    current->size = offset - sizeof(heap_header_t);
                    current->next = aligned_block;
                    current = aligned_block;
                }
                return allocate_block(current, size);
            }
        }
        current = current->next;
    }

    KLOG(LOG_LEVEL_ERROR, "[heap] ERROR: Aligned allocation failed\n");
    return NULL;
}

/**
 * Frees memory allocated by kmalloc/kmalloc_aligned.
 *
 * @param ptr Pointer to memory block to free.
 */
void kfree(void* ptr) {
    if (!ptr) return;

    heap_header_t* header = (heap_header_t*)((u8*)ptr - sizeof(heap_header_t));
    header->is_free = 1;

    KLOG(LOG_LEVEL_DEBUG, "[heap] Memory freed at 0x%X (%u bytes)\n", (u32)ptr, header->size);

    // Coalesce adjacent free blocks
    heap_header_t* current = free_list;
    while (current) {
        if (current->is_free && current->next && current->next->is_free) {
            current->size += sizeof(heap_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

/**
 * Reallocates a memory block with a new size.
 *
 * @param ptr       Pointer to current memory block.
 * @param old_size  Original size of the block.
 * @param new_size  Desired new size.
 * @return Pointer to newly allocated memory or NULL on failure.
 */
void* krealloc(void* ptr, u32 old_size, u32 new_size) {
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) return NULL;

    // Copy old data to new block
    for (u32 i = 0; i < old_size && i < new_size; i++) {
        ((char*)new_ptr)[i] = ((char*)ptr)[i];
    }

    // Free old block
    kfree(ptr);

    return new_ptr;
}

