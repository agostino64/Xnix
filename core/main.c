/*
 * Xnix
 *
 * Copyright (C) 2022, 2025  Agustin Gutierrez
 */

#include <stdint.h>
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

extern uint32_t placement_address;
uint32_t initial_esp;

/**
 * start_kernel - Entry point for the Xnix kernel after boot.
 *
 * This function initializes core subsystems in order:
 * 1. Descriptor tables (GDT, IDT)
 * 2. Serial driver for logging
 * 3. Memory: paging and heap
 * 4. Initial RAM disk (initrd)
 * 5. Timer and keyboard drivers
 * 6. Preemptive multitasking
 *
 */
void start_kernel(uint32_t initial_stack, struct multiboot *mboot_ptr)
{
    clear_screen();
    printk("Xnix Booting...\n\n");
    initial_esp = initial_stack;

    // Initialize core components
    serial_init();
    KLOG(LOG_LEVEL_INFO, "Initializing serial driver...\n");
    
    init_descriptor_tables();
    KLOG(LOG_LEVEL_INFO, "Initializing GDT/IDT...\n");

    // Enable interrupts early for timer and keyboard
    KLOG(LOG_LEVEL_INFO, "Enabling interrupts...\n");
    sti();

    // Verify and locate initrd
    if (mboot_ptr->mods_count == 0) {
        panic(NULL, "No modules found (initrd missing)!");
    }
    uint32_t initrd_location = *(uint32_t*)mboot_ptr->mods_addr;
    uint32_t initrd_end      = *(uint32_t*)(mboot_ptr->mods_addr + 4);
    placement_address   = initrd_end;

    // Initialize Paging & Heap
    KLOG(LOG_LEVEL_INFO, "Initializing paging...\n");
    uint32_t mem_bytes = (mboot_ptr->mem_lower + mboot_ptr->mem_upper) * 1024;
    init_paging(mem_bytes);
    KLOG(LOG_LEVEL_DEBUG, "Paging initialized (%d MB)\n", mem_bytes / (1024 * 1024));

    // Load Filesystem
    KLOG(LOG_LEVEL_INFO, "Loading initrd...\n");
    fs_root = initialise_initrd(initrd_location);

    // Initialize Drivers
    KLOG(LOG_LEVEL_INFO, "Initializing timer driver...\n");
    init_timer(); // Note: init_timer is now separate from serial_init

    KLOG(LOG_LEVEL_INFO, "Initializing keyboard driver...\n");
    init_keyboard();

    // Launch the preemptive scheduler
    KLOG(LOG_LEVEL_INFO, "Launching preemptive multitasking...\n");
    init_tasking();
    
    // The scheduler is now running. The first yield will switch to the shell.
    //yield();
    
    // This code will only run when the scheduler gives control back to the main task.
    KLOG(LOG_LEVEL_DEBUG, "Returned to main kernel task.\n");
    
    for (int i=0;i<10;i++)
      KLOG(LOG_LEVEL_DEBUG, "We are on main task.\n");
    
    // Nothing to do... sleep
    for (;;) { __asm__ __volatile__("hlt"); }
}

void print_task(void)
{
  KLOG(LOG_LEVEL_INFO, "Hello from print task!\n");
}

