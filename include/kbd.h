#pragma once

#include <stdint.h>
#include <stddef.h>

#define BUFFER_SIZE 256

uint8_t kbd_interrupt(uint8_t *keycode);
void kbd_init(char *buffer, size_t buffer_size);
