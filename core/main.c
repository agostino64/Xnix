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
#include <xnix/drv_manager.h>
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
void start_kernel(struct multiboot *mboot_ptr, u32 initial_stack)
{
    printk("Xnix Kernel Booting...\n\n");
    initial_esp = initial_stack;

    // -------------------------------
    // Step 1: Set up CPU descriptor tables
    // -------------------------------
    printk("Initializing GDT/IDT...\n");
    init_descriptor_tables();

    // -------------------------------
    // Step 2: Initialize serial driver
    // -------------------------------
    printk("Initializing serial driver...\n");
    if (drv_load(DRV_SERIAL) != 0) {
        KLOG(LOG_LEVEL_ERROR, "Failed to load DRV_SERIAL\n");
    }

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
    printk("Initializing paging...\n");
    u32 mem_bytes = (mboot_ptr->mem_lower + mboot_ptr->mem_upper) * 1024;
    init_paging(mem_bytes);
    KLOG(LOG_LEVEL_INFO, "Paging initialized (%d MB)\n", mem_bytes / (1024 * 1024));

    // Memory allocation test
    u32 malloc_test = kmalloc(100);
    if (malloc_test != 0) {
        KLOG(LOG_LEVEL_INFO, "Heap test successful: allocated 100 bytes at 0x%x\n", (u32)malloc_test);
        kfree((void*)malloc_test);
    } else {
        KLOG(LOG_LEVEL_ERROR, "Heap test failed: kmalloc returned NULL\n");
    }

    // -------------------------------
    // Step 6: Load initrd filesystem
    // -------------------------------
    printk("Loading initrd...\n");
    fs_root = initialise_initrd(initrd_location);

    // -------------------------------
    // Step 7: Initialize timer driver
    // -------------------------------
    printk("Initializing timer driver...\n");
    if (drv_load(DRV_TIMER) != 0) {
        KLOG(LOG_LEVEL_ERROR, "Failed to load DRV_TIMER\n");
    }

    // -------------------------------
    // Step 8: Initialize keyboard driver
    // -------------------------------
    printk("Initializing keyboard driver...\n");
    if (drv_load(DRV_KEYBOARD) != 0) {
        KLOG(LOG_LEVEL_ERROR, "Failed to load DRV_KEYBOARD\n");
    }

    printk("Launching tasking...\n");
    initTasking(); // Task init on mainTask and switch to shellTask

    printk("Shell is launched...\n");
    
    yield(); // Switch to mainTask
    
    KLOG(LOG_LEVEL_INFO, "Not in shell task!\n");
    yield(); // Switch to shellTask
    
    // Should never return; fallback in case shell exits
    while (1) {
        __asm__ __volatile__("cli; hlt");
    }
}

