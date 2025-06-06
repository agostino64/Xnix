#ifndef PANIC_H
#define PANIC_H

#include "common.h"
#include "isr.h"

void panic(registers_t* regs, const char* message) __attribute__((noreturn));
void panic_assert(const char *file, u32 line, const char* message) __attribute__((noreturn));

#endif

