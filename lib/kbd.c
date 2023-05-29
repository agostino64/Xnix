#include <stdbool.h>
#include <kbd.h>
#include <screen.h>
#include <libstr.h>
#include <io.h>

// mapa de teclas del teclado
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  0,			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

// Buffer del teclado
static uint32_t keyboard_buffer_pos = 0;

char kbd_interrupt(unsigned char *data)
{ 
  unsigned char status;
  *data = 0;
  
  while (1)
  {
    status = i686_inb(0x64);
    if (status & 0x01)
    {
      *data = i686_inb(0x60);
      break;
    }
  }
  
  if (*data >= sizeof(kbdus) / sizeof(kbdus[0]))
  {
    return 0;
  }

  return kbdus[*data];	
}

// Función para leer una línea completa del teclado
void kbd_interrupt_handler(char *buffer, size_t buffer_size)
{
    unsigned char keycode;
    keyboard_buffer_pos = 0;
    
    while (1)
    {
        char c = kbd_interrupt(&keycode);

        // print keys on pressed
        if (c) 
          putc(c, 0x7);

        if (c == '\n')
        {
            buffer[keyboard_buffer_pos] = '\0';
            return;
        }
        else if (c >= ' ' && keyboard_buffer_pos < buffer_size - 1)
        {
            buffer[keyboard_buffer_pos] = c;
            keyboard_buffer_pos++;
        }
        else if (keyboard_buffer_pos >= buffer_size - 1)
        {
            // buffer overflow detected
            buffer[buffer_size - 1] = '\0';
            return;
        }
    }
}

void kbd_init(char *buffer, size_t buffer_size)
{
  kbd_interrupt_handler(buffer, buffer_size);
}
