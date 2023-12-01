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

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start(uint16_t bootDrive)
{   
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    clear_vga();
    
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
    
    init_tty();
}
