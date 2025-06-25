// timer.h -- Defines the interface for all PIT-related functions.
// Written for JamesM's kernel development tutorials.

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "../common.h"

void timer_wait(uint32_t ticks);
void init_timer(void);

#endif
