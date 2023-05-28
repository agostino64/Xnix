#include <screen.h>
#include <libstr.h>
#include <kbd.h>

void start_console(char *k_input)
{  
    kprint_color(">> ", 0x4);
    kbd_init(k_input, BUFFER_SIZE);
    if (k_strcmp(k_input, "") == 0)
      start_console(k_input);
}

void kmain()
{
    char k_input[BUFFER_SIZE];
    
    kclear_screen();
    
    kprint_color("Welcome to XnixOS!\n\n", 0x3);
    kprint("Write \'help\'\n");
    while(1)
    {
      start_console(k_input);
      if (k_strcmp(k_input, "shutdown") == 0)
          kprint("hello!\n");
      else if (k_strcmp(k_input, "reboot") == 0)
          kprint("reiniciar!\n");
      else if (k_strcmp(k_input, "clear") == 0)
          kclear_screen();
      else 
          kprint("!\n");
    }
}
