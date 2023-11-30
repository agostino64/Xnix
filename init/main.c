/*
 *  XnixOS
 *
 *  xnix/init/main.c
 */

#include <xnix/vga.h>
#include <xnix/common.h>
#include <xnix/keyb.h>
#include <xnix/isr.h>
#include <xnix/timer.h>
#include <xnix/asm.h>
#include <xnix/multiboot.h>
#include <xnix/shell.h>
#include <xnix/gdt.h>

uint32_t initial_esp;
static char* cmd;

void start_kernel(struct multiboot *mbd, uint32_t initial_stack)
{
    initial_esp = initial_stack; //Set up for stack switch
    
    clear_vga();
    
    cli(); // dissable interrupts
    
    isr_init();
    init_gdt();
    printk("IDT/GDT initialised...\n");
    
    /* IRQ0: timer */
    init_timer(50);
    printk("CMOS timer initialised to 50hz...\n");
    
    /* IRQ1: keyboard */
    init_keyboard();
    printk("PS/2 keyboard initialised...\n\n");
    
    sti(); // enable interrupts
    
    init_tty(cmd);
}
