#include <xnix/vga.h>
#include <xnix/drivers/keyb.h>

void printf(const char *msg, ...) { 
    va_list args;
    va_start(args, msg);
    vprintk(msg, args);
    va_end(args);
}
char* scanf() {
    char *input = get_input_buffer();
    gets();
    return input;
}