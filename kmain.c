#include <screen.h>
#include <libstr.h>
#include <kbd.h>

#include <hal.h>

void start_console(char *k_input)
{  
    printk(">> ");
    kbd_init(k_input, BUFFER_SIZE);
    if (k_strcmp(k_input, "") == 0)
      start_console(k_input);
}

void kmain()
{
    char k_input[BUFFER_SIZE];
    
    HAL_Initialize();
    
    kclear_screen();

    printk("Welcome to XnixOS!\n\n");
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
