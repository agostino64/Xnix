/*
 * timer.c -- Initializes the PIT and handles clock ticks.
 *
 * Adapted from JamesM's kernel development tutorials.
 */

#include <stdint.h>
#include <xnix/drivers/timer.h>
#include <xnix/isr.h>
#include <xnix/vga.h>
#include <xnix/common.h>
#include <xnix/log.h>
#include <xnix/task.h> // Required for scheduling

#define FREQUENCY 50  // Target frequency in Hz (50Hz = 20ms tick)
#define TIME_SLICE 4  // Switch tasks every 4 ticks (~80ms timeslice)

uint32_t tick = 0;
volatile uint32_t wait_ticks = 0;

// The schedule function is defined in task.c
extern void schedule(registers_t *regs);

/**
 * timer_callback - IRQ0 handler, called on each PIT tick.
 * @regs: register snapshot from the interrupt frame.
 */
static void timer_callback(registers_t *regs)
{
    tick++;
    wait_ticks++;
    
    // When a time slice expires, call the scheduler to switch tasks.
    if (tick % TIME_SLICE == 0) {
        schedule(regs);
    }
}

/**
 * timer_wait - Busy-wait for a number of timer ticks.
 * @ticks: number of ticks to wait (at 50Hz, 50 ticks = 1 second)
 */
void timer_wait(uint32_t ticks)
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
    // Register IRQ0 handler for preemptive multitasking.
    // Note: The isr_t type now takes a pointer to registers_t.
    register_interrupt_handler(IRQ0, (isr_t)timer_callback);
    KLOG(LOG_LEVEL_INFO, "Registered IRQ0 handler for preemptive multitasking.\n");

    // Calculate divisor for PIT (1193180 Hz input clock)
    uint32_t divisor = 1193180 / FREQUENCY;

    // Send command byte to PIT
    outb(0x43, 0x36);  // Channel 0, LSB/MSB, mode 3, binary
    KLOG(LOG_LEVEL_DEBUG, "Sending PIT command byte: 0x36\n");

    // Send divisor LSB then MSB
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);

    KLOG(LOG_LEVEL_INFO, "PIT initialized to %u Hz (divisor=%u)\n", FREQUENCY, divisor);
}
