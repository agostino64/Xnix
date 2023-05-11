#pragma once

#include <stddef.h>
#include <stdint.h>
#include <hal/vfs.h>

#define KEYBOARD_STATUS_PORT 0x64 // RW
#define KEYBOARD_DATA_PORT 0x60 // RONLY

#define BACKSPACE 0x08
#define DELETE 0x7F

#define BUFFER_SIZE 256

char kbd_interrupt(unsigned char *data);
void kbd_init(char *buffer, size_t buffer_size);
