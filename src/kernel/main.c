#include <stdint.h>
#include <stdio.h>
#include "libstr.h"
#include <hal/hal.h>

#include <drivers/kbd.h>
#include <arch/i686/vga_text.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void crash_me();

void start_console(char *k_input)
{  
    printf(">> ");
    kbd_init(k_input, BUFFER_SIZE);
    if (k_strcmp(k_input, "") == 0)
      start_console(k_input);
}

void __attribute__((section(".entry"))) start(uint16_t bootDrive)
{
    k_memset(&__bss_start, 0, (&__end) - (&__bss_start));
    
    HAL_Initialize();
    
    char k_input[BUFFER_SIZE];
    
    printf("Welcome to XnixOS!\n\n");
    printf("Write \'help\'\n");
    while(1)
    {
      start_console(k_input);
      if (k_strcmp(k_input, "shutdown") == 0)
          printf("apagar!\n");
      else if (k_strcmp(k_input, "reboot") == 0)
          printf("reiniciar!\n");
      else 
          commands(k_input);
    }

end:
    for (;;);
}
