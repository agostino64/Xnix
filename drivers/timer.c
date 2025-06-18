/*
 *  timer.c -- Initializes the PIT and handles clock ticks.
 *
 *  Adapted from JamesM's kernel development tutorials.
 */

#include <xnix/drivers/timer.h>
#include <xnix/isr.h>
#include <xnix/vga.h>
#include <xnix/common.h>
#include <xnix/log.h>

#define FREQUENCY 50  // Target frequency in Hz (50Hz = 20ms tick)

u32 tick = 0;
volatile u32 wait_ticks = 0;

/**
 * timer_callback - IRQ0 handler, called on each PIT tick.
 * @regs: register snapshot (unused)
 */
static void timer_callback(registers_t regs)
{
    tick++;
    wait_ticks++;
    
    if (tick % FREQUENCY == 0) {
        //KLOG(LOG_LEVEL_DEBUG, "Timer tick %u (1s elapsed)\n", tick);
    }
}

/**
 * timer_wait - Busy-wait for a number of timer ticks.
 * @ticks: number of ticks to wait (at 50Hz, 50 ticks = 1 second)
 */
void timer_wait(u32 ticks)
{
    wait_ticks = 0;
    KLOG(LOG_LEVEL_DEBUG, "Waiting for %u ticks\n", ticks);
    while (wait_ticks <= ticks);
    KLOG(LOG_LEVEL_DEBUG, "Finished waiting %u ticks\n", ticks);
}

/**
 * init_timer - Initializes the PIT (Programmable Interval Timer).
 *
 * Registers the IRQ0 timer handler and configures the PIT for the defined
 * frequency (default: 50Hz).
 *
 * Return: 0 on success
 */
void init_timer(void)
{
    // Register IRQ0 handler
    register_interrupt_handler(IRQ0, &timer_callback);
    KLOG(LOG_LEVEL_INFO, "Registered IRQ0 handler for system timer.\n");

    // Calculate divisor for PIT (1193180 Hz input clock)
    u32 divisor = 1193180 / FREQUENCY;

    // Send command byte to PIT
    outb(0x43, 0x36);  // Channel 0, LSB/MSB, mode 3, binary
    KLOG(LOG_LEVEL_DEBUG, "Sending PIT command byte: 0x36\n");

    // Send divisor LSB then MSB
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);

    KLOG(LOG_LEVEL_INFO, "PIT initialized to %u Hz (divisor=%u)\n", FREQUENCY, divisor);
}
