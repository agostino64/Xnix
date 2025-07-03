//
// isr.c -- High level interrupt service routines and interrupt request handlers.
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//

#include <stdint.h>
#include <xnix/common.h>
#include <xnix/isr.h>
#include <xnix/vga.h>
#include <xnix/log.h>
#include <xnix/cpu.h>
#include <xnix/panic.h>
#include <xnix/task.h> // For schedule()

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
  interrupt_handlers[n] = handler;
} 

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t *regs)
{
    //KLOG(LOG_LEVEL_DEBUG, "[ISR] Interrupt %d received (EIP=0x%X, err=0x%X)\n", regs->int_no, regs->eip, regs->err_code);

    // Special handling for the yield system call (INT 0x80 / 13)
    if (regs->int_no == 13) {
        schedule(regs);
        return;
    }

    // If a custom handler is registered, dispatch to it
    if (interrupt_handlers[regs->int_no]) {
        KLOG(LOG_LEVEL_DEBUG,
             "[ISR] Dispatching custom handler for INT %d\n",
             regs->int_no);
        interrupt_handlers[regs->int_no](regs);
    } else {
        KLOG(LOG_LEVEL_ERROR,
             "[ISR] Unhandled exception INT %d at EIP=0x%X, err=0x%X\n",
             regs->int_no, regs->eip, regs->err_code);
        panic(regs, "CPU exception");
    }
}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t *regs)
{
   // Send an EOI (end of interrupt) signal to the PICs.
   if (regs->int_no >= 40)
   {
       // Send reset signal to slave.
       outb(0xA0, 0x20);
   }
   // Send reset signal to master.
   outb(0x20, 0x20);

   if (interrupt_handlers[regs->int_no] != 0)
   {
       isr_t handler = interrupt_handlers[regs->int_no];
       handler(regs); // Call the handler if one exists
   }
}
