/*
 *  Xnix
 *  VGA Text-mode Driver
 *  
 *  Copyright (C) 2022, 2025  Agustin Gutierrez
 */

#include <stdint.h>
#include <xnix/common.h>
#include <xnix/drivers/serial.h>

// Color definitions (foreground: black, background: light grey)
#define FOREGROUND 0
#define BACKGROUND 7

#define TAB_SIZE 4

// VGA framebuffer starts at memory location 0xB8000
uint16_t *video_memory = (uint16_t*)0xb8000;

// Cursor position
uint32_t cursor_y, cursor_x;

/**
 * move_cursor - Updates the hardware cursor location using VGA ports.
 */
static void move_cursor(void)
{
    uint16_t cursorLocation = cursor_y * 80 + cursor_x;
    
    outb(0x3D4, 14);                  // Set high byte
    outb(0x3D5, cursorLocation >> 8);
    outb(0x3D4, 15);                  // Set low byte
    outb(0x3D5, cursorLocation);
}

/**
 * scroll - Scrolls the VGA display up by one line if cursor exceeds bottom.
 */
static void scroll(void)
{
    uint8_t attributeByte = (FOREGROUND << 4) | (BACKGROUND & 0x0F);
    uint16_t blank = 0x20 | (attributeByte << 8); // space character with attribute

    if (cursor_y >= 25)
    {
        for (int32_t i = 0; i < 24 * 80; i++)
            video_memory[i] = video_memory[i + 80];

        for (int32_t i = 24 * 80; i < 25 * 80; i++)
            video_memory[i] = blank;

        cursor_y = 24;
    }
}

/**
 * put - Writes a single character to the VGA screen with special handling.
 */
void put(const char c)
{
    uint8_t attributeByte = (FOREGROUND << 4) | (BACKGROUND & 0x0F);
    uint16_t attribute = attributeByte << 8;
    uint16_t *location;

    switch (c)
    {
        case '\b': // Backspace
            if (cursor_x != 0) {
                cursor_x--;
                location = video_memory + (cursor_y * 80 + cursor_x);
                *location = ' ' | attribute;
             }
            break;

        case '\t': // Tab
            cursor_x += TAB_SIZE;
            break;

        case '\r': // Carriage return
            cursor_x = 0;
            break;

        case '\n': // Newline
            cursor_x = 0;
            cursor_y++;
            break;

        default: // Printable characters
            if (c >= ' ')
            {
                location = video_memory + (cursor_y * 80 + cursor_x);
                *location = c | attribute;
                cursor_x++;
            }
            break;
    }

    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y++;
    }

    scroll();
    move_cursor();
}

/**
 * clear_screen - Clears the entire screen with blank spaces.
 */
void clear_screen(void)
{
    uint8_t attributeByte = (FOREGROUND << 4) | (BACKGROUND & 0x0F);
    uint16_t blank = 0x20 | (attributeByte << 8);

    for (int32_t i = 0; i < 80 * 25; i++)
        video_memory[i] = blank;

    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

/**
 * write - Outputs a null-terminated string to the screen.
 */
void write(const char *c)
{
    for (int32_t i = 0; c[i]; i++)
        put(c[i]);
}

/* ---------------- printk Implementation ---------------- */

// State machine for format parsing
#define PRINTK_STATE_NORMAL         0
#define PRINTK_STATE_LENGTH         1
#define PRINTK_STATE_LENGTH_SHORT   2
#define PRINTK_STATE_LENGTH_LONG    3
#define PRINTK_STATE_SPEC           4

// Length specifiers
#define PRINTK_LENGTH_DEFAULT       0
#define PRINTK_LENGTH_SHORT_SHORT   1
#define PRINTK_LENGTH_SHORT         2
#define PRINTK_LENGTH_LONG          3

const char hexchars[] = "0123456789abcdef";

/**
 * printk_unsigned - Converts an unsigned long to a string and prints it.
 */
void printk_unsigned(unsigned long number, int32_t radix)
{
    char buffer[32];
    int32_t pos = 0;

    do
    {
        buffer[pos++] = hexchars[number % radix];
        number /= radix;
    } while (number > 0);

    while (--pos >= 0)
        put(buffer[pos]);
}

/**
 * printk_signed - Handles signed long numbers (adds '-' if needed).
 */
void printk_signed(long number, int32_t radix)
{
    if (number < 0)
    {
        put('-');
        printk_unsigned(-number, radix);
    }
    else
        printk_unsigned(number, radix);
}

/**
 * vprintk - Core formatted print function using va_list.
 */
void vprintk(const char* fmt, va_list args)
{
    int32_t state = PRINTK_STATE_NORMAL;
    int32_t length = PRINTK_LENGTH_DEFAULT;
    int32_t radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTK_STATE_NORMAL:
                if (*fmt == '%')
                    state = PRINTK_STATE_LENGTH;
                else
                    put(*fmt);
                break;

            case PRINTK_STATE_LENGTH:
                if (*fmt == 'h')
                {
                    length = PRINTK_LENGTH_SHORT;
                    state = PRINTK_STATE_LENGTH_SHORT;
                }
                else if (*fmt == 'l')
                {
                    length = PRINTK_LENGTH_LONG;
                    state = PRINTK_STATE_LENGTH_LONG;
                }
                else
                    goto PRINTK_STATE_SPEC_;
                break;

            case PRINTK_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = PRINTK_LENGTH_SHORT_SHORT;
                    state = PRINTK_STATE_SPEC;
                }
                else
                    goto PRINTK_STATE_SPEC_;
                break;

            case PRINTK_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = PRINTK_LENGTH_LONG;
                    state = PRINTK_STATE_SPEC;
                }
                else
                    goto PRINTK_STATE_SPEC_;
                break;

            case PRINTK_STATE_SPEC:
            PRINTK_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c':   put((char)va_arg(args, int32_t)); break;
                    case 's':   write((char*)va_arg(args, const char*)); break;
                    case '%':   put('%'); break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true; break;
                    case 'u':   radix = 10; sign = false; number = true; break;
                    case 'x':
                    case 'X':
                    case 'p':   radix = 16; sign = false; number = true; break;
                    case 'o':   radix = 8;  sign = false; number = true; break;

                    default:    break; // ignore unsupported format
                }

                if (number)
                {
                    if (sign)
                    {
                        if (length == PRINTK_LENGTH_LONG)
                            printk_signed(va_arg(args, long), radix);
                        else
                            printk_signed(va_arg(args, int32_t), radix);
                    }
                    else
                    {
                        if (length == PRINTK_LENGTH_LONG)
                            printk_unsigned(va_arg(args, unsigned long), radix);
                        else
                            printk_unsigned(va_arg(args, uint32_t), radix);
                    }
                }

                // Reset for next sequence
                state = PRINTK_STATE_NORMAL;
                length = PRINTK_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }

        fmt++;
    }
}

/**
 * printk - Kernel-level formatted print function (like printf).
 */
void printk(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
