/*
 *  Xnix
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  Implements x86 paging with dynamic allocation of page tables.
 *  Provides basic identity mapping and virtual heap support using slab-style allocation.
 *  Enables page-level memory isolation and supports runtime expansion of page tables.
 */
 
#include <xnix/paging.h>
#include <xnix/heap.h>
#include <xnix/common.h>
#include <xnix/vga.h>
#include <xnix/log.h>

// Page flags for access control and status
#define PAGE_PRESENT    0x1    // Page is present in memory
#define PAGE_RW         0x2    // Page is writable
#define PAGE_USER       0x4    // Page is accessible from user mode (not used here)

// Paging structure base addresses
#define KERNEL_PAGE_DIR 0x9C000        // Physical address of the page directory

// Pointer to the page directory
static u32* page_directory = (u32*)KERNEL_PAGE_DIR;

// Slab-style pool of pre-allocated page tables
#define MAX_PAGE_TABLES 16
static u32* page_table_pool[MAX_PAGE_TABLES];
static int next_free_pt = 0;

// Allocates a new page table from the slab pool
static u32* alloc_page_table(void) {
    if (next_free_pt >= MAX_PAGE_TABLES) return NULL;
    u32* pt = (u32*)(0x9D000 + next_free_pt * PAGE_SIZE);
    memset(pt, 0, PAGE_SIZE);
    page_table_pool[next_free_pt++] = pt;
    return pt;
}

// Enables paging by setting CR3 and CR0 register values
void enable_paging(void) {
    __asm__ __volatile__ ("mov %0, %%cr3" :: "r"(page_directory));
    u32 cr0;
    __asm__ __volatile__ ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Set paging enable bit
    __asm__ __volatile__ ("mov %0, %%cr0" :: "r"(cr0));
}

// Maps a single virtual address to a physical address
void map_page(u32 virtual_addr, u32 physical_addr) {
    u32 pd_idx = PAGE_DIRECTORY_INDEX(virtual_addr);
    u32 pt_idx = PAGE_TABLE_INDEX(virtual_addr);

    u32* page_table;
    if (page_directory[pd_idx] & PAGE_PRESENT) {
        page_table = (u32*)(page_directory[pd_idx] & ~0xFFF);
    } else {
        page_table = alloc_page_table();
        if (!page_table) {
            KLOG(LOG_LEVEL_ERROR, "[paging] ERROR: Out of page table slabs\n");
            return;
        }
        page_directory[pd_idx] = ((u32)page_table) | PAGE_PRESENT | PAGE_RW;
    }

    page_table[pt_idx] = (physical_addr & ~0xFFF) | PAGE_PRESENT | PAGE_RW;
}

// Maps a range of contiguous virtual addresses to physical addresses
void map_range(u32 vaddr_start, u32 paddr_start, u32 size) {
    for (u32 offset = 0; offset < size; offset += PAGE_SIZE) {
        map_page(vaddr_start + offset, paddr_start + offset);
    }
}

// Initializes paging, sets up identity and heap mappings, and enables paging
void init_paging(void) {
    memset(page_directory, 0, PAGE_SIZE);

    // Identity map 0x00000000 to 0x003FFFFF (4MB)
    map_range(0x00000000, 0x00000000, 0x400000);

    // Map virtual heap address to physical memory
    map_range(HEAP_VIRT_ADDR, HEAP_PHYS_ADDR, HEAP_SIZE);

    KLOG(LOG_LEVEL_DEBUG, "[paging] Page directory at 0x%X\n", (u32)page_directory);
    KLOG(LOG_LEVEL_DEBUG, "[paging] Mapped heap: 0xC0000000 -> 0x%X (%u bytes)\n", HEAP_PHYS_ADDR, HEAP_SIZE);

    enable_paging();
    KLOG(LOG_LEVEL_DEBUG, "[paging] Paging enabled.\n");
}

