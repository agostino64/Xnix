/*
 *  Xnix
 *
 *  Copyright (C) 2022, 2025  Agustin Gutierrez
 */

#include <xnix/vga.h>
#include <xnix/descriptor_tables.h>
#include <xnix/timer.h>
#include <xnix/keyb.h>
#include <xnix/common.h>
#include <xnix/cpu.h>
#include <xnix/shell.h>
#include <xnix/isr.h>
#include <xnix/heap.h>
#include <xnix/paging.h>

/**
 * start_kernel - Entry point for the Xnix kernel after boot.
 * 
 * This function initializes core subsystems in order:
 * - Screen
 * - Interrupt Descriptor Table (IDT) & Global Descriptor Table (GDT)
 * - Virtual Memory Paging
 * - Kernel Heap
 * - Interrupts (enabled via STI)
 * - Timer (set to 50Hz)
 * - Keyboard Driver
 * - Interactive Shell
 * 
 * It ends in an infinite loop that halts the CPU.
 * 
 * Note: __attribute__((noreturn)) indicates this function never returns.
 */
__attribute__((noreturn)) void start_kernel(void)
{     
    clear_screen();
    printk("Loading...\n");

    // Set up GDT and IDT for protected mode and interrupt handling
    printk("Init IDT/GDT...\n");
    init_descriptor_tables();
    
    // Initialize virtual memory and enable paging
    printk("Init paging...\n");
    init_paging();  // Sets up basic page tables and enables paging
    
    // Set up dynamic memory allocation (kernel heap)
    printk("Init heap...\n");
    init_heap();
    
    // Enable interrupts globally (sets IF flag)
    printk("Enable interrupts...\n");
    sti();
    
    // Start the programmable interval timer at 50 Hz (20ms tick)
    printk("Init Timer...\n");
    init_timer(50);
    
    // Set up keyboard interrupt handler and input buffer
    printk("Init keyboard...\n");
    init_keyboard();
 
    // Launch the interactive shell for user input
    printk("Init shell...\n");
    init_shell();
    
    // Infinite loop to halt the CPU when idle
    while (1) {
        halt();
    }
}

