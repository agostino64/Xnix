#ifndef PAGING_H
#define PAGING_H

#include "common.h"

#define PAGE_SIZE 4096
#define PAGE_DIRECTORY_INDEX(addr) (((addr) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(addr)     (((addr) >> 12) & 0x3FF)

void init_paging(void);
void enable_paging(void);
void map_page(u32 virtual_addr, u32 physical_addr);
void map_range(u32 vaddr_start, u32 paddr_start, u32 size);

#endif

