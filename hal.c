#include <hal.h>
#include <gdt.h>
#include <idt.h>

void HAL_Initialize()
{
    i686_GDT_Initialize();
    i686_IDT_Initialize();
}
