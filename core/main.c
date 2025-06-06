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

u32 initial_esp;

/**
 * start_kernel - Entry point for the Xnix kernel after boot.
 * 
 * This function initializes core subsystems in order:
 * - Screen
 * - Interrupt Descriptor Table (IDT) & Global Descriptor Table (GDT)
 * - Virtual Memory Paging
 * - Kernel Heap
 * - Interrupts (enabled via STI)
 * - Timer (default set to 50Hz)
 * - Keyboard Driver
 * - Interactive Shell
 * 
 * It ends in an infinite loop that halts the CPU.
 * 
 * Note: __attribute__((noreturn)) indicates this function never returns.
 */
__attribute__((noreturn)) void start_kernel(struct multiboot *mboot_ptr, u32 initial_stack)
{     
    printk("Xnix...\n\n");
    initial_esp = initial_stack;

    // Set up GDT and IDT for protected mode and interrupt handling
    printk("Init IDT/GDT...\n");
    init_descriptor_tables();
    
    printk("Init serial driver...\n");
    if (drv_load(DRV_SERIAL) != 0) {
        KLOG(LOG_LEVEL_ERROR, "DRV_SERIAL load failed!\n");
    }
    
    //Initialise paging with the memory amount reported by GRUB
    init_paging();
    printk("Paging initialised\n");

    
    // Set up dynamic memory allocation (kernel heap)
    printk("Init heap...\n");
    init_heap();
    
    // Enable interrupts globally (sets IF flag)
    printk("Enable interrupts...\n");
    sti();
    
    // Start the programmable interval timer at 50 Hz (20ms tick)
    printk("Init Timer driver...\n");
    if (drv_load(DRV_TIMER) != 0) {
        KLOG(LOG_LEVEL_ERROR, "DRV_TIMER load failed!\n");
    }
    
    // Set up keyboard interrupt handler and input buffer
    printk("Init keyboard driver...\n");
    if (drv_load(DRV_KEYBOARD) != 0) {
        KLOG(LOG_LEVEL_ERROR, "DRV_KEYBOARD load failed!\n");
    }
 
    // Launch the interactive shell for user input
    printk("Init shell...\n");
    init_shell();
    
    while (1)
    {
      halt();
    }
}

