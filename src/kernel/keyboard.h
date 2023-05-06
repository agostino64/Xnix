#pragma once

#include <stddef.h>
#include <stdint.h>

#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60
#define BUFFER_SIZE 1024

void keyboard_read_line(char *buffer, size_t buffer_size);
unsigned char inb(unsigned short port);

