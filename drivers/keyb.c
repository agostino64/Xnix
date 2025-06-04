/*
 *  Xnix
 *
 *  Copyright (C) 2022, 2025  Agustin Gutierrez
 *
 *  Basic keyboard driver for handling PS/2 input.
 *  Supports scancode decoding, buffering input characters, 
 *  and processing Caps Lock / Shift modifiers.
 */

#include <xnix/common.h>
#include <xnix/isr.h>
#include <xnix/vga.h>
#include <xnix/drv_register.h>
#include <xnix/drv_control.h>

extern u32 cursor_y, cursor_x;

// Modifier flags
volatile int shift_flag = 0;
volatile int caps_flag = 0;

// Buffers for capturing user input
volatile char* buffer;   // Current input buffer
volatile char* buffer2;  // Copy for gets()
volatile int kb_count = 0; // Index for input buffer
volatile int gets_flag = 0; // Flag indicating newline (Enter) received

// LED control bits for keyboard
unsigned short ltmp;
int ktmp = 0;

// Internal function to transfer input buffer to gets buffer
static void do_gets(void);

/**
 * US QWERTY keymap for scancode set 1.
 * Values of 1 are reserved or unprintable.
 */
unsigned char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',       // Backspace
    '\t',                           // Tab
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', // Enter
    1,                              // Control
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,                   // Left Shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 1,         // Right Shift
    '*',
    1,                              // Alt
    ' ',                            // Spacebar
    1,                              // Caps Lock
    1, 1, 1, 1, 1, 1, 1, 1,         // F1–F8
    1,                              // F9
    1,                              // Num Lock
    1,                              // Scroll Lock
    1,                              // Home
    1,                              // Up
    1,                              // Page Up
    '-', 1, 1,                      // Left, ??, Right
    '+', 1,                         // End
    1,                              // Down
    1,                              // Page Down
    1,                              // Insert
    1,                              // Delete
    1, 1, '\\',                     // ??, ??, Backslash
    1, 1,                           // F11, F12
    1                               // All others undefined
};

/**
 * Keyboard interrupt handler.
 * Processes scancodes, manages buffer input, and handles modifier states.
 */
static void keyboard_handler(registers_t regs)
{
    unsigned char scancode = inb(0x60);

    switch (scancode)
    {
        case 0x3A: // Caps Lock
            outb(0x60, 0xED); // Prepare LED change
            ltmp |= 4;        // Caps bit
            outb(0x60, ltmp);
            caps_flag = !caps_flag;
            break;

        case 0x45: // Num Lock
            outb(0x60, 0xED);
            ltmp |= 2;
            outb(0x60, ltmp);
            break;

        case 0x46: // Scroll Lock
            outb(0x60, 0xED);
            ltmp |= 1;
            outb(0x60, ltmp);
            break;

        case 60: // F12 (placeholder for reboot or other feature)
            // reboot();
            break;

        default:
            break;
    }

    if (scancode & 0x80) {
        // Key release (bit 7 set)
        if (scancode - 0x80 == 42 || scancode - 0x80 == 54)
            shift_flag = 0; // Left/Right Shift released
    } else {
        // Key press
        if (scancode == 42 || scancode == 54) {
            shift_flag = 1;
            return;
        }

        // Enter key
        if (kbdus[scancode] == '\n') {
            if (gets_flag == 0) do_gets(); // Transfer buffer
            gets_flag++;
            while (kb_count) buffer[kb_count--] = 0; // Clear buffer
        }
        else {
            if (kbdus[scancode] == '\b') {
                if (kb_count) buffer[kb_count--] = 0;
            } else {
                buffer[kb_count++] = kbdus[scancode];
            }
        }

        put(kbdus[scancode]); // Echo character to screen
        return;
    }
}

/**
 * Registers the keyboard interrupt handler (IRQ1).
 * Call this once during system initialization.
 */
static int init_keyboard(void)
{
    register_interrupt_handler(IRQ1, &keyboard_handler);
    return 0;
}

/**
 * Reads a single character from keyboard buffer.
 * Waits until a key is available.
 *
 * @return ASCII character code of key pressed.
 */
unsigned char getch(void)
{
    unsigned char getch_char;

    if (kbdus[inb(0x60)] != 0)
        outb(0x60, 0xF4); // Clear buffer

    while (kbdus[inb(0x60)] == 0); // Wait for input
    getch_char = kbdus[inb(0x60)];
    outb(0x60, 0xF4); // Acknowledge

    return getch_char;
}

/**
 * Reads a line of input from the keyboard until Enter is pressed.
 * Returns pointer to the static input buffer.
 *
 * @return Null-terminated input string.
 */
char* gets(void)
{
    gets_flag = 0;
    while (gets_flag == 0);
    return (char*)buffer2;
}

/**
 * Transfers the current buffer content to buffer2.
 * Called internally when Enter is pressed.
 */
static void do_gets(void)
{
    buffer[kb_count++] = 0; // Null-terminate

    for (; kb_count; kb_count--)
        buffer2[kb_count] = buffer[kb_count];
}

/**
 * Returns pointer to latest keyboard input buffer (copied on Enter).
 *
 * @return Pointer to static buffer2.
 */
char* get_input_buffer(void)
{
    return (char*)buffer2;
}

REGISTER_DRIVER(init_keyboard, DRV_KEYBOARD);
