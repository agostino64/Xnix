#include <xnix/vga.h>
#include <xnix/descriptor_tables.h>
#include <xnix/timer.h>
#include <xnix/keyb.h>
#include <xnix/common.h>
#include <xnix/cpu.h>
#include <xnix/shell.h>
#include <xnix/isr.h>

void start_kernel(void)
{     
    printk("Loading...\n");
    // Initialise all the ISRs and segmentation
    printk("Init IDT/GDT...\n");
    init_descriptor_tables();
    
    // Enable interrupts
    printk("Enable Interrupts...\n");
    sti();
    
    // Initialise timer to 50Hz 
    printk("Init Timer...\n");
    init_timer(50);
    
    printk("Init Keyboard...\n");
    init_keyboard();
 
    printk("Init Shell...\n");
    init_shell();
}
