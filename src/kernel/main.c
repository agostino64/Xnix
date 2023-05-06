#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include "keyboard.h"
#include "string.h"
#include <hal/hal.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void crash_me();

void start_console(char *k_input)
{  
    printf("-> ");
    keyboard_read_line(k_input, BUFFER_SIZE);
    if (k_strcmp(k_input, "") == 0)
      start_console(k_input);
}

void __attribute__((section(".entry"))) start(uint16_t bootDrive)
{
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    
    HAL_Initialize();
    
    printf("Welcome to tOS!\n\n");
    
    char k_input[BUFFER_SIZE];
    
    while(1)
    {
      start_console(k_input);
      if (k_strcmp(k_input, "shutdown") == 0)
          shutdown_hw();
      else if (k_strcmp(k_input, "reboot") == 0)
          reboot_hw();
      else 
          commands(k_input);
    }

end:
    for (;;);
}
