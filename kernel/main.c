#include <screen.h>
#include <libstr.h>
#include <kbd.h>
#include <isr.h>
#include <timer.h>
#include <asm.h>
#include <multiboot.h>
#include <shell.h>
#include <gdt.h>

uint32_t initial_esp;
static char* cmd;

void start_kernel(struct multiboot *mbd, uint32_t initial_stack)
{
    initial_esp = initial_stack; //Set up for stack switch
    cli(); // dissable interrupts
    
    kclear_screen();
    
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
