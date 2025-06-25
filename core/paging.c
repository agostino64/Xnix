// paging.c -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#include <stdint.h>
#include <xnix/paging.h>
#include <xnix/heap.h>
#include <xnix/common.h>
#include <xnix/vga.h>
#include <xnix/log.h>
#include <xnix/cpu.h>
#include <xnix/panic.h>
#include <xnix/task.h>
#include <xnix/memory.h>
#include <xnix/string.h>

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory;
page_directory_t *current_directory=0;

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;

volatile uint32_t memsize = 0;

// Defined in kheap.c
extern uint32_t placement_address;
extern heap_t *kheap;

// Defined in process.asm
extern void copy_page_physical(unsigned int src,unsigned int page);

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
    //KLOG(LOG_LEVEL_DEBUG, "Set frame: 0x%X (bit %d in index %d)\n", frame_addr, off, idx);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
    KLOG(LOG_LEVEL_DEBUG, "Cleared frame: 0x%X (bit %d in index %d)\n", frame_addr, off, idx);
}

// Static function to find the first free frame.
static uint32_t first_frame(void)
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
        if (frames[i] != 0xFFFFFFFF)
        {
            for (j = 0; j < 32; j++)
            {
                uint32_t toTest = 0x1 << j;
                if ( !(frames[i]&toTest) )
                {
                    //KLOG(LOG_LEVEL_DEBUG, "First free frame: %d\n", i*4*8+j);
                    return i*4*8+j;
                }
            }
        }
    }
    KLOG(LOG_LEVEL_ERROR, "No free frames available!\n");
    return (uint32_t)-1;
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
    if (page->frame != 0)
        return;
    else
    {
        uint32_t idx = first_frame();
        if (idx == (uint32_t)-1)
        {
            //panic("no free frames!!");
        }
        set_frame(idx*0x1000);
        page->present = 1;
        page->rw = (is_writeable==1)?1:0;
        page->user = (is_kernel==1)?0:1;
        page->frame = idx;
        //KLOG(LOG_LEVEL_DEBUG, "Allocated frame: %u to page entry\n", idx);
    }
}

void free_frame(page_t *page)
{
    uint32_t frame;
    if (!(frame=page->frame))
        return;
    else
    {
        clear_frame(frame);
        page->frame = 0x0;
        KLOG(LOG_LEVEL_DEBUG, "Freed frame: 0x%X\n", frame);
    }
}

void init_paging(unsigned int memorysz)
{
    // The size of physical memory.
    uint32_t mem_end_page = memorysz;
    memsize = memorysz;
    
    nframes = mem_end_page / 0x1000;
    frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
    memset((uint8_t*)frames, 0, INDEX_FROM_BIT(nframes));
    
    // Let's make a page directory.
    //uint32_t phys;
    kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
    memset((uint8_t*)kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = (uint32_t)kernel_directory->tablesPhysical;

    // Map some pages in the kernel heap area.
    // Here we call get_page but not alloc_frame. This causes page_table_t's 
    // to be created where necessary. We can't allocate frames yet because they
    // they need to be identity mapped first below, and yet we can't increase
    // placement_address between identity mapping and enabling the heap!
    int i = 0;
    for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000)
        get_page(i, 1, kernel_directory);

    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    // NOTE that we use a while loop here deliberately.
    // inside the loop body we actually change placement_address
    // by calling kmalloc(). A while loop causes this to be
    // computed on-the-fly rather than once at the start.
    // Allocate a lil' bit extra so the kernel heap can be
    // initialised properly.
    i = 0;
    while (i < placement_address+0x1000)
    {
        // Kernel code is readable but not writeable from userspace.
        alloc_frame( get_page(i, 1, kernel_directory), 1, 0);
        i += 0x1000;
    }

    // Now allocate those pages we mapped earlier.
    for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000)
        alloc_frame( get_page(i, 1, kernel_directory), 1, 0);

    
    // Before we enable paging, we must register our page fault handler.
    register_interrupt_handler(14, &page_fault);

    // Now, enable paging!
    switch_page_directory(kernel_directory);

    // Initialise the kernel heap.
    kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, KHEAP_START+KHEAP_MAX_ADDRESS, 0, 0);
    expand(0x400000, kheap); // Allocate some more space
    
    current_directory = clone_directory(kernel_directory);
    switch_page_directory(current_directory);
}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    __asm__ __volatile__("mov %0, %%cr3":: "r"(dir->physicalAddr));
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    __asm__ __volatile__("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir)
{
    // Turn the address into an index.
    address /= 0x1000;
    // Find the page table containing this address.
    uint32_t table_idx = address / 1024;

    if (dir->tables[table_idx]) // If this table is already assigned
    {
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else if(make)
    {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
        memset((uint8_t*)dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
        return &dir->tables[table_idx]->pages[address%1024];
    }
    else
    {
        return 0;
    }
}


void page_fault(registers_t regs)
{
    __asm__ __volatile__ ("cli");
    
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));
    
    // The error code gives us details of what happened.
    int present   = !(regs.err_code & 0x1); // Page not present
    int rw = regs.err_code & 0x2;           // Write operation?
    int us = regs.err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    //int id = regs.err_code & 0x10;          // Caused by an instruction fetch?
    
    uint32_t cr2;
    __asm__ __volatile__("mov %%cr2, %0": "=r"(cr2));

    // Output an error message.
    printk("Page fault! (");
    if (present) {printk("page not present ");}
    if (rw) {printk("read-only ");}
    if (us) {printk("user-mode ");}
    if (reserved) {printk("reserved ");}
    printk("\b) at 0x%x - EIP: %x \n",faulting_address,regs.eip);
    
    if(!strcmp((char*)regs.eip,(char*)cr2))
    	printk("Page fault caused by executing unpaged memory\n");
    else
    	printk("Page fault caused by reading unpaged memory\n");
    
    
    if(current_directory != kernel_directory)
    {
    	//printk("Killing task %d\n",getpid());
    	panic(&regs, "Task fault");
    }
    //else
    	//PANIC("Page fault");
}

static page_table_t *clone_table(page_table_t *src, uint32_t *physAddr)
{
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddr);
    // Ensure that the new table is blank.
    memset((uint8_t*)table, 0, sizeof(page_table_t));

    // For every entry in the table...
    int i;
    for (i = 0; i < 1024; i++)
    {
        // If the source entry has a frame associated with it...
        if (src->pages[i].frame)
        {
            // Get a new frame.
            alloc_frame(&table->pages[i], 0, 0);
            // Clone the flags from source to destination.
            if (src->pages[i].present) table->pages[i].present = 1;
            if (src->pages[i].rw) table->pages[i].rw = 1;
            if (src->pages[i].user) table->pages[i].user = 1;
            if (src->pages[i].accessed) table->pages[i].accessed = 1;
            if (src->pages[i].dirty) table->pages[i].dirty = 1;
            // Physically copy the data across. This function is in process.s.
            copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
        }
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src)
{
    uint32_t phys;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    // Ensure that it is blank.
    memset((uint8_t*)dir, 0, sizeof(page_directory_t));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    uint32_t offset = (uint32_t)dir->tablesPhysical - (uint32_t)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            uint32_t phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}

void map_pages(long addr, long size, int rw, int user)
{
    long i = addr;
    while (i < (addr+size+0x1000))
    {
         if (i < memsize) {
              set_frame(i); // Tell the frame bitmap that this frame is now used!
              page_t *page = get_page(i, 1, current_directory);
              page->present = 1;
              page->rw = rw;
              page->user = user;
              page->frame = i / 0x1000;
         }
         else {
             KLOG(LOG_LEVEL_ERROR, "map_pages: skipping physical address 0x%X (beyond memsize=0x%X)\n", i, memsize);
         }
         i += 0x1000;
    }
    return;
}

void virtual_map_pages(long addr, long size, int rw, int user)
{
    long i = addr;
    while (i < (addr+size+0x1000))
    {
        page_t *page = get_page(i, 1, current_directory);
        if (!page) {
            KLOG(LOG_LEVEL_ERROR, "virtual_map_pages: get_page failed for 0x%X\n", i);
            break;
        }
        // user: 1 means user-accessible, so we set is_kernel=0. user:0 means kernel-only, so is_kernel=1.
        alloc_frame(page, (user==1)?0:1, rw);
        if (page->frame == 0) {
            KLOG(LOG_LEVEL_ERROR, "virtual_map_pages: alloc_frame failed for 0x%X\n", i);
            break;
        }
        i += 0x1000;
    }
    return;
}

void* alloc_task_stack_page(void) {
    static uint32_t next_stack_address = ADDR_PAGE_TASK;

    page_t *page = get_page(next_stack_address, 1, current_directory);
    if (!page) {
        KLOG(LOG_LEVEL_ERROR, "get_page failed for task stack at 0x%X", next_stack_address);
        return 0;
    }

    alloc_frame(page, 0, 1); // is_kernel=0 (user), is_writeable=1

    void* result = (void*)next_stack_address;
    next_stack_address += 0x1000; // Go to the next page
    KLOG(LOG_LEVEL_INFO, "[Paging] Allocated task stack page at %p\n", result);
    return result;
}
