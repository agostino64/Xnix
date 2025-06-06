/*
 *  Xnix
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  Panic Handler
 *  Provides functions to handle critical errors in the kernel.
 *  Halts the system after printing detailed CPU state.
 */

#include <xnix/common.h>
#include <xnix/log.h>
#include <xnix/drivers/serial.h>
#include <xnix/vga.h>
#include <xnix/isr.h>

/**
 * panic - Triggers a kernel panic with a given error message and CPU state.
 *
 * @param message  The reason for the panic.
 * @param regs     Pointer to a structure containing CPU register values.
 *
 * This function logs the error, prints register values, and halts the CPU.
 * It does not return.
 */
__attribute__((noreturn))
void panic(registers_t* regs, const char* message) 
{
    printk("\n\n*** KERNEL PANIC ***\n");
    printk("Reason: %s\n", message);
    printk("System Halted.\n\n");

    if (regs) {
        // Print CPU register state
        KLOG(LOG_LEVEL_INFO, "==== Registers Dump ====\n");
        KLOG(LOG_LEVEL_INFO, " EAX: 0x%08x  EBX: 0x%08x  ECX: 0x%08x  EDX: 0x%08x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
        KLOG(LOG_LEVEL_INFO, " ESI: 0x%08x  EDI: 0x%08x  EBP: 0x%08x  ESP: 0x%08x\n", regs->esi, regs->edi, regs->ebp, regs->esp);
        KLOG(LOG_LEVEL_INFO, " EIP: 0x%08x  CS:  0x%08x  EFLAGS: 0x%08x\n", regs->eip, regs->cs, regs->eflags);
        KLOG(LOG_LEVEL_INFO, " USERESP: 0x%08x  SS: 0x%08x\n", regs->useresp, regs->ss);
        KLOG(LOG_LEVEL_INFO, " INT#: %u  ERR_CODE: 0x%08x\n", regs->int_no, regs->err_code);
        KLOG(LOG_LEVEL_INFO, "========================\n");
    } else {
        KLOG(LOG_LEVEL_INFO, "No register state provided.\n");
    }

    // Halt the system
    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}

/**
 * panic_assert - Triggers a kernel panic from a failed assertion.
 *
 * @param file     Source file where the assertion failed.
 * @param line     Line number of the assertion.
 * @param message  Assertion message.
 * @param regs     CPU state at the time of the failure.
 *
 * This function logs debug info and halts the system.
 */
__attribute__((noreturn))
void panic_assert(const char *file, u32 line, const char* message) 
{
    printk("\n\n*** KERNEL PANIC ***\n");
    printk("Reason: %s\n", message);
    printk("File: %s\n", file);
    printk("Line: %d\n", line);
    printk("System Halted.\n\n");

    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}

