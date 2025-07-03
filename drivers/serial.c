/*
 *  Xnix - Serial Driver
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  This module implements low-level support for COM1 serial communication.
 *  It includes initialization of the UART 16550A-compatible controller,
 *  character output via polling, interrupt-driven input handling,
 *  and a minimal formatted `printk_serial` interface.
 *
 *  Supported Features:
 *    - COM1 initialization (38400 baud, 8N1)
 *    - Character and string output (polling)
 *    - IRQ4-based interrupt handler for input
 *    - Registering a user callback for serial input
 *    - Formatted output with `printk_serial()` and `vprintk_serial()`
 */

#include <stdint.h>
#include <xnix/drivers/serial.h>
#include <xnix/common.h>
#include <xnix/isr.h>

#define COM1_PORT 0x3F8

// Optional: callback to be called on received char
static void (*serial_input_callback)(char) = 0;

/**
 * serial_is_transmit_empty - Check if the transmit buffer is empty.
 * 
 * Return: 1 if the serial port is ready to send a character.
 */
int serial_is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

/**
 * serial_write_char - Write a single character to the serial port.
 * @a: character to transmit
 *
 * Blocks until the transmit buffer is available.
 */
void serial_write_char(char a) {
    while (!serial_is_transmit_empty());
    outb(COM1_PORT, a);
}

/**
 * serial_write_string - Write a null-terminated string to the serial port.
 * @str: pointer to the string to transmit
 */
void serial_write_string(const char* str) {
    while (*str) {
        if (*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

/**
 * serial_read_char - Read a character from the serial port.
 *
 * Return: the byte read from the receive buffer.
 */
char serial_read_char(void) {
    return inb(COM1_PORT);
}

/**
 * serial_irq_handler - IRQ4 handler for COM1 serial input.
 * @regs: CPU register snapshot at the time of the interrupt
 *
 * Reads the incoming character and calls the registered callback, if any.
 */
void serial_irq_handler(registers_t *regs) {
    uint8_t status = inb(COM1_PORT + 5);
    if (status & 1) {
        char c = serial_read_char();
        if (serial_input_callback) {
            serial_input_callback(c);
        }
    }
}

/**
 * serial_init - Initialize the COM1 serial port (0x3F8).
 *
 * Sets baud rate to 38400, enables FIFO, 8N1 format, and IRQs.
 * Registers IRQ4 handler to handle serial input.
 *
 * Return: 0 on success
 */
void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable interrupts during setup
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB
    outb(COM1_PORT + 0, 0x03);    // Set baud rate divisor to 3 (38400 baud)
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear it
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set

    // Enable "received data available" interrupt
    outb(COM1_PORT + 1, 0x01);

    // Register IRQ4 handler (COM1)
    register_interrupt_handler(IRQ4, &serial_irq_handler);
    
}

/**
 * serial_set_input_callback - Register a function to be called on input.
 * @callback: function pointer of type `void callback(char c)`
 *
 * When a character is received over COM1, this callback is invoked.
 */
void serial_set_input_callback(void (*callback)(char)) {
    serial_input_callback = callback;
}

// State machine for format parsing
#define PRINTK_SERIAL_STATE_NORMAL         0
#define PRINTK_SERIAL_STATE_LENGTH         1
#define PRINTK_SERIAL_STATE_LENGTH_SHORT   2
#define PRINTK_SERIAL_STATE_LENGTH_LONG    3
#define PRINTK_SERIAL_STATE_SPEC           4

// Length specifiers
#define PRINTK_SERIAL_LENGTH_DEFAULT       0
#define PRINTK_SERIAL_LENGTH_SHORT_SHORT   1
#define PRINTK_SERIAL_LENGTH_SHORT         2
#define PRINTK_SERIAL_LENGTH_LONG          3

const char hexchars_serial[] = "0123456789abcdef";

/**
 * printk_serial_unsigned - Print an unsigned number in the given base.
 * @number: value to print
 * @radix: base (e.g., 10 for decimal, 16 for hex)
 */
void printk_serial_unsigned(unsigned long number, int32_t radix) {
    char buffer[32];
    int32_t pos = 0;

    do {
        buffer[pos++] = hexchars_serial[number % radix];
        number /= radix;
    } while (number > 0);

    while (--pos >= 0)
        serial_write_char(buffer[pos]);
}

/**
 * printk_serial_signed - Print a signed number in the given base.
 * @number: value to print
 * @radix: base (e.g., 10 for decimal, 16 for hex)
 */
void printk_serial_signed(long number, int32_t radix) {
    if (number < 0) {
        serial_write_char('-');
        printk_serial_unsigned(-number, radix);
    } else {
        printk_serial_unsigned(number, radix);
    }
}

/**
 * vprintk_serial - Formatted output via serial using va_list.
 * @fmt: format string (supports %d, %u, %x, %s, %c, etc.)
 * @args: variable argument list
 *
 * Implements a lightweight printf-style formatter and outputs via serial.
 */
void vprintk_serial(const char* fmt, va_list args)
{
    int32_t state = PRINTK_SERIAL_STATE_NORMAL;
    int32_t length = PRINTK_SERIAL_LENGTH_DEFAULT;
    int32_t radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTK_SERIAL_STATE_NORMAL:
                if (*fmt == '%')
                    state = PRINTK_SERIAL_STATE_LENGTH;
                else
                    serial_write_char(*fmt);
                break;

            case PRINTK_SERIAL_STATE_LENGTH:
                if (*fmt == 'h') {
                    length = PRINTK_SERIAL_LENGTH_SHORT;
                    state = PRINTK_SERIAL_STATE_LENGTH_SHORT;
                } else if (*fmt == 'l') {
                    length = PRINTK_SERIAL_LENGTH_LONG;
                    state = PRINTK_SERIAL_STATE_LENGTH_LONG;
                } else goto PRINTK_SERIAL_STATE_SPEC_;
                break;

            case PRINTK_SERIAL_STATE_LENGTH_SHORT:
                if (*fmt == 'h') {
                    length = PRINTK_SERIAL_LENGTH_SHORT_SHORT;
                    state = PRINTK_SERIAL_STATE_SPEC;
                } else goto PRINTK_SERIAL_STATE_SPEC_;
                break;

            case PRINTK_SERIAL_STATE_LENGTH_LONG:
                if (*fmt == 'l') {
                    length = PRINTK_SERIAL_LENGTH_LONG;
                    state = PRINTK_SERIAL_STATE_SPEC;
                } else goto PRINTK_SERIAL_STATE_SPEC_;
                break;

            case PRINTK_SERIAL_STATE_SPEC:
            PRINTK_SERIAL_STATE_SPEC_:
                switch (*fmt) {
                    case 'c':   serial_write_char((char)va_arg(args, int32_t)); break;
                    case 's':   serial_write_string((char*)va_arg(args, const char*)); break;
                    case '%':   serial_write_char('%'); break;
                    case 'd':
                    case 'i': radix = 10; sign = true; number = true; break;
                    case 'u': radix = 10; sign = false; number = true; break;
                    case 'x':
                    case 'X':
                    case 'p': radix = 16; sign = false; number = true; break;
                    case 'o': radix = 8;  sign = false; number = true; break;

                    default: break;
                }

                if (number)
                {
                    if (sign)
                    {
                        if (length == PRINTK_SERIAL_LENGTH_LONG)
                            printk_serial_signed(va_arg(args, long), radix);
                        else
                            printk_serial_signed(va_arg(args, int32_t), radix);
                    }
                    else
                    {
                        if (length == PRINTK_SERIAL_LENGTH_LONG)
                            printk_serial_unsigned(va_arg(args, unsigned long), radix);
                        else
                            printk_serial_unsigned(va_arg(args, uint32_t), radix);
                    }
                }

                state = PRINTK_SERIAL_STATE_NORMAL;
                length = PRINTK_SERIAL_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }

        fmt++;
    }

}

/**
 * printk_serial - Print formatted output to serial (like printf).
 * @fmt: format string with optional specifiers
 * @...: variadic arguments matching the format specifiers
 *
 * Supports standard types (%d, %u, %x, %s, %c, %%).
 */
void printk_serial(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintk_serial(fmt, args);
    va_end(args);
}

extern const char* log_get_timestamp(void);
void printk_serial_timetamp(const char* fmt, ...)
{
    serial_write_string(log_get_timestamp());
    va_list args;
    va_start(args, fmt);
    vprintk_serial(fmt, args);
    va_end(args);
}

