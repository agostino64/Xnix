#include "keyboard.h"
#include "stdio.h"
#include "memory.h"

unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0  /* Control */,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0 /* Left shift */,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0 /* Right shift */, '*',
    0 /* Alt */, ' ',   0 /* Caps lock */,   0 /* F1 */,   0 /* F2 */,   0 /* F3 */,   0 /* F4 */,   0 /* F5 */,
    0 /* F6 */,   0 /* F7 */,   0 /* F8 */,   0 /* F9 */,   0 /* F10 */,  0 /* Num lock */,  0 /* Scroll lock */,
    0 /* Home */,  0 /* Up arrow */, 0 /* Page up */, '-', 0 /* Left arrow */, '5', 0 /* Right arrow */, '+',
     0 /* End */,   0 /* Down arrow */, 0 /* Page down */, 0 /* Insert */, 0 /* Delete */, 0, 0, 0 /* F11 */, 0 /* F12 */
};

char keyboard_buffer[BUFFER_SIZE];
uint32_t keyboard_buffer_pos = 0;

char keycode_to_ascii(unsigned char keycode) {
    if (keycode >= sizeof(keyboard_map) / sizeof(keyboard_map[0])) {
        return 0;
    }
    return keyboard_map[keycode];
}

char keyboard_get_char() {
    unsigned char keycode;
    do {
        keycode = inb(KEYBOARD_STATUS_PORT);
    } while ((keycode & 0x01) == 0);
    keycode = inb(KEYBOARD_DATA_PORT);

    char c = keycode_to_ascii(keycode);

    if (c) {
        printf("%c", c);
    }
    
    return c;
}

void keyboard_read_line(char *buffer, size_t buffer_size)
{
    keyboard_buffer_pos = 0;
    while (1)
    {
        char c = keyboard_get_char();
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
    }
}

