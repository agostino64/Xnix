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

// from core/version.c
extern const char *get_xnix_version(void);
extern const char *get_xnix_build_date(void);
extern const char *get_xnix_build_time(void);

// Function prototypes
void start_kernel_core(struct multiboot *mboot_ptr);
void kernel_loop(void);

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
    initial_esp = initial_stack;
    
    start_kernel_core(mboot_ptr);

    clear_screen();

    printk("Xnix Kernel %s started successfully.\n\n", get_xnix_version());
    KLOG(LOG_LEVEL_INFO, "Xnix Kernel %s started successfully.\n", get_xnix_version());
    KLOG(LOG_LEVEL_INFO, "Build date: %s at %s\n", get_xnix_build_date(), get_xnix_build_time());

    kernel_loop();
}

void start_kernel_core(struct multiboot *mboot_ptr)
{
    printk("Xnix Booting...\n\n");

    // Initialize core components
    printk("Initializing serial driver...");
    serial_init();
    printk("OK.\n");

    printk("Initializing GDT/IDT...");
    init_descriptor_tables();
    printk("OK.\n");

    // Enable interrupts early for timer and keyboard
    printk("Enabling interrupts...");
    sti();
    printk("enabled.\n");

    // Verify and locate initrd
    if (mboot_ptr->mods_count == 0) {
        panic(NULL, "No modules found (initrd missing)!");
    }
    uint32_t initrd_location = *(uint32_t*)mboot_ptr->mods_addr;
    uint32_t initrd_end      = *(uint32_t*)(mboot_ptr->mods_addr + 4);
    placement_address   = initrd_end;

    // Initialize Paging & Heap
    printk("Initializing paging...");
    uint32_t mem_bytes = (mboot_ptr->mem_lower + mboot_ptr->mem_upper) * 1024;
    init_paging(mem_bytes);
    printk("OK.\n");
    KLOG(LOG_LEVEL_DEBUG, "Paging initialized (%d MB)\n", mem_bytes / (1024 * 1024));

    // Load Filesystem
    printk("Loading initrd...");
    fs_root = initialise_initrd(initrd_location);
    printk("OK.\n");
    KLOG(LOG_LEVEL_DEBUG, "Initrd loaded at %p\n", fs_root);

    // Initialize Drivers
    printk("Initializing timer driver...");
    init_timer(); // Note: init_timer is now separate from serial_init
    printk("OK.\n");

    printk("Initializing keyboard driver...");
    init_keyboard();
    printk("OK.\n");

    // Launch the preemptive scheduler
    printk("Launching preemptive multitasking...");
    init_tasking();
    printk("OK.\n");

    KLOG(LOG_LEVEL_DEBUG, "core components initialized.\n");
}

void kernel_loop(void)
{
    // Nothing to do... sleep
    for (;;) {
      halt();
     }
}

void print_task(void)
{
    KLOG(LOG_LEVEL_INFO, "Hello from print task!\n");
}
