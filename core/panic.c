/*
 *  XnixOS
 *
 *  xnix/core/panic.c
 */
 
#include <xnix/vga.h>
#include <xnix/isr.h>
#include <xnix/asm.h>
#include <xnix/util.h>
   
void panic(registers_t* regs)
{
     beep();
     printk("\n\n\t\tPANIC!\n\n");
     printk("EIP: 0x%x ESP: 0x%x EBP: 0x%x\n",regs->eip,regs->esp,regs->ebp);
     printk("EDI: 0x%x ESI: 0x%x\n",regs->edi,regs->esi,regs->ebp);
     printk("EAX: 0x%x EBX: 0x%x ECX: 0x%x EDX: 0x%x\n",regs->eax,regs->ebx,regs->ecx,regs->edx);
     printk("CS: 0x%x DS: 0x%x SS: 0x%x\n",regs->cs,regs->ds,regs->ss);
     printk("INT: 0x%x ERROR: 0x%x\n",regs->int_no,regs->err_code);
     cli();
     halt();
}

void panic_assert(const char *file, uint32_t line, const char *desc)
{
     printk("Assertion failed (%d) at %s:%d\n",desc,file,line);
}
