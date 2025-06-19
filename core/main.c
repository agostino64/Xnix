/*
 *  Xnix
 *
 *  Copyright (C) 2022, 2025  Agustin Gutierrez
 */

#include <xnix/vga.h>
#include <xnix/descriptor_tables.h>
#include <xnix/drivers/timer.h>
#include <xnix/drivers/keyb.h>
#include <xnix/common.h>
#include <xnix/cpu.h>
#include <xnix/shell.h>
#include <xnix/isr.h>
#include <xnix/heap.h>
#include <xnix/paging.h>
#include <xnix/log.h>
#include <xnix/multiboot.h>
#include <xnix/drivers/serial.h>
#include <xnix/fs.h>
#include <xnix/initrd.h>
#include <xnix/panic.h>
#include <xnix/task.h>

extern u32 placement_address;
u32 initial_esp;

/**
 * start_kernel - Entry point for the Xnix kernel after boot.
 *
 * This function initializes core subsystems in order:
 *  1. Descriptor tables (GDT, IDT)
 *  2. Serial driver for logging
 *  3. Memory: paging and heap
 *  4. Initial RAM disk (initrd)
 *  5. Timer and keyboard drivers
 *  6. Shell
 *
 */
void start_kernel(u32 initial_stack, struct multiboot *mboot_ptr)
{
    clear_screen();     // ← now safe: stack is 16-byte aligned
    printk("Xnix Booting...\n\n");
    initial_esp = initial_stack;

    // -------------------------------
    // Step 2: Initialize serial driver
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Initializing serial driver...\n");
    serial_init();

    // -------------------------------
    // Step 1: Set up CPU descriptor tables
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Initializing GDT/IDT...\n");
    init_descriptor_tables();

    // -------------------------------
    // Step 3: Enable interrupts
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Enabling interrupts...\n");
    sti();

    // -------------------------------
    // Step 4: Verify and locate initrd
    // -------------------------------
    if (mboot_ptr->mods_count == 0) {
        panic(NULL, "No modules found (initrd missing)!");
    }

    u32 initrd_location = *(u32*)mboot_ptr->mods_addr;
    u32 initrd_end      = *(u32*)(mboot_ptr->mods_addr + 4);
    placement_address   = initrd_end;  // Avoid overwriting initrd

    // -------------------------------
    // Step 5: Initialize paging & heap
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Initializing paging...\n");
    u32 mem_bytes = (mboot_ptr->mem_lower + mboot_ptr->mem_upper) * 1024;
    init_paging(mem_bytes);
    KLOG(LOG_LEVEL_DEBUG, "Paging initialized (%d MB)\n", mem_bytes / (1024 * 1024));

    // Memory allocation test
    u32 malloc_test = kmalloc(100);
    if (malloc_test != 0) {
        KLOG(LOG_LEVEL_DEBUG, "Heap test successful: allocated 100 bytes at 0x%x\n", (u32)malloc_test);
        kfree((void*)malloc_test);
    } else {
        KLOG(LOG_LEVEL_ERROR, "Heap test failed: kmalloc returned NULL\n");
    }

    // -------------------------------
    // Step 6: Load initrd filesystem
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Loading initrd...\n");
    fs_root = initialise_initrd(initrd_location);

    // -------------------------------
    // Step 7: Initialize timer driver
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Initializing timer driver...\n");
    serial_init();

    // -------------------------------
    // Step 8: Initialize keyboard driver
    // -------------------------------
    KLOG(LOG_LEVEL_INFO, "Initializing keyboard driver...\n");
    init_keyboard();

    KLOG(LOG_LEVEL_INFO, "Launching tasking...\n");
    initTasking(); // Task init on mainTask and switch to shellTask

    KLOG(LOG_LEVEL_DEBUG, "Shell is launched...\n");    
    yield(); // Switch to shellTask
    
    // now in mainTask
    KLOG(LOG_LEVEL_DEBUG, "Not in shell task!\n");
    yield(); // Switch to shellTask
    
    // Should never return; fallback in case shell exits
    while (1) {
        yield(); // switch between tasks
    }
}

