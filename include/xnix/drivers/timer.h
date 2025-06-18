// timer.h -- Defines the interface for all PIT-related functions.
// Written for JamesM's kernel development tutorials.

#ifndef TIMER_H
#define TIMER_H

#include "../common.h"

void timer_wait(u32 ticks);
void init_timer(void);

#endif
