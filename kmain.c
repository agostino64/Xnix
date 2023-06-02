#include <screen.h>
#include <libstr.h>
#include <kbd.h>
#include <cmd.h>

#include <hal.h>

void kmain()
{
    char k_input[BUFFER_SIZE];
    
    HAL_Initialize();
    
    kclear_screen();

    puts("Welcome to XnixOS!\n\n", 0x3);
    while(1)
    {
      start_console(k_input);
      if (k_strcmp(k_input, "shutdown") == 0)
          printk("apagar! %s\n", k_input);
      else if (k_strcmp(k_input, "reboot") == 0)
          printk("reiniciar!\n");
      else if (k_strcmp(k_input, "panic") == 0)
          printk("Kernel in panic!\n");
      else
          cmd_init(k_input);
    }
}
