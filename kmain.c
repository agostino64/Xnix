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
    
    init_tty(k_input);
}
