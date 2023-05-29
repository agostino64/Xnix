#pragma once

#include <stdint.h>
#include <stddef.h>

#define BUFFER_SIZE 256

char kbd_interrupt(unsigned char *data);
void kbd_init(char *buffer, size_t buffer_size);

void cmd_init(char *k_input);
