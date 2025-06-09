/*
 *  Xnix
 *
 *  Copyright (C) 2023, 2025  Agustin Gutierrez
 *
 *  Enhanced keyboard driver for PS/2 input.
 *  Features:
 *    - Fixed scancode decoding with US QWERTY keymap
 *    - Linear input buffer with explicit capture on Enter
 *    - Proper Caps Lock/Shift handling
 *    - Efficient LED state management
 *    - Echo of characters to VGA console
 */

#include <xnix/common.h>
#include <xnix/isr.h>
#include <xnix/vga.h>
#include <xnix/drv_register.h>
#include <xnix/drv_control.h>
#include <xnix/log.h>
#include <xnix/task.h>
#include <xnix/cpu.h>    // for inb(), outb()

// Modifier flags
static volatile int shift_flag = 0;
static volatile int caps_flag  = 0;
static volatile int ctrl_flag  = 0;  // reserved for future use

// LED state byte
static unsigned char kbd_led_state = 0;

// Input buffers and state
#define KB_BUFFER_SIZE 256
static volatile char buffer[KB_BUFFER_SIZE];   // raw input buffer
static volatile char buffer2[KB_BUFFER_SIZE];  // captured line buffer
static volatile int  kb_count    = 0;  // number of chars in 'buffer'
static volatile int  gets_flag   = 0;  // set when Enter is pressed

// US QWERTY keymaps
static const unsigned char kbdus_norm[128] = {
    /* 0x00 - 0x0F */    0,  0x1B,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    /* 0x10 - 0x1F */  'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0 ,'a','s',
    /* 0x20 - 0x2F */  'd','f','g','h','j','k','l',';','\'', '`', 0 ,'\\','z','x','c','v',
    /* 0x30 - 0x3F */  'b','n','m',',','.','/', 0 ,'*', 0 ,' ',' ', 0 , 0 , 0 , 0 , 0 ,
    /* 0x40 - 0x4F */    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,'7','8','9','-',
    /* 0x50 - 0x5F */  '4','5','6','+','1','2','3','0','.', 0 , 0 , 0 , 0 , 0 , 0 , 0
};

static const unsigned char kbdus_shift[128] = {
    /* 0x00 - 0x0F */    0,  0x1B,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
    /* 0x10 - 0x1F */  'Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0 ,'A','S',
    /* 0x20 - 0x2F */  'D','F','G','H','J','K','L',':','"','~', 0 ,'|','Z','X','C','V',
    /* 0x30 - 0x3F */  'B','N','M','<','>','?', 0 ,'*', 0 ,' ',' ', 0 , 0 , 0 , 0 , 0 ,
    /* 0x40 - 0x4F */    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,'7','8','9','-',
    /* 0x50 - 0x5F */  '4','5','6','+','1','2','3','0','.', 0 , 0 , 0 , 0 , 0 , 0 , 0
};

// Scancode definitions
#define SC_LSHIFT    0x2A
#define SC_RSHIFT    0x36
#define SC_CAPSLOCK  0x3A
#define SC_NUMLOCK   0x45
#define SC_SCROLLOCK 0x46
#define SC_ENTER     0x1C
#define SC_BACKSPACE 0x0E
#define SC_F12       0x58

// Wait until PS/2 controller is ready, then send LED update commands
static void update_leds(void) {
    while (inb(0x64) & 0x02) { /* wait */ }
    outb(0x60, 0xED);  // command: set LEDs
    while (inb(0x64) & 0x02) { /* wait */ }
    outb(0x60, kbd_led_state);
}

// Copy raw buffer -> public buffer2 and NUL-terminate
static void do_gets(void) {
    int len = (kb_count < KB_BUFFER_SIZE-1) ? kb_count : (KB_BUFFER_SIZE-1);
    for (int i = 0; i < len; i++) {
        buffer2[i] = buffer[i];
    }
    buffer2[len] = '\0';
}

// PS/2 keyboard interrupt handler
static void keyboard_handler(registers_t regs) {
    unsigned char scancode = inb(0x60);

    // Ignore key releases
    if (scancode & 0x80) {
        unsigned char release = scancode & 0x7F;
        if (release == SC_LSHIFT || release == SC_RSHIFT) {
            shift_flag = 0;
        }
        return;
    }

    if (scancode == SC_LSHIFT || scancode == SC_RSHIFT) {
        shift_flag = 1;
        return;
    }
    if (scancode == SC_CAPSLOCK) {
        caps_flag = !caps_flag;
        kbd_led_state = (kbd_led_state & ~0x04) | (caps_flag ? 0x04 : 0);
        update_leds();
        KLOG(LOG_LEVEL_DEBUG, "Caps Lock %s\n", caps_flag ? "ON" : "OFF");
        return;
    }
    if (scancode == SC_NUMLOCK) {
        kbd_led_state ^= 0x02;
        update_leds();
        KLOG(LOG_LEVEL_DEBUG, "Num Lock toggled\n");
        return;
    }
    if (scancode == SC_SCROLLOCK) {
        kbd_led_state ^= 0x01;
        update_leds();
        KLOG(LOG_LEVEL_DEBUG, "Scroll Lock toggled\n");
        return;
    }
    if (scancode == SC_F12) {
        KLOG(LOG_LEVEL_INFO, "F12 pressed\n");
        return;
    }

    // Decode character
    char key = shift_flag ? kbdus_shift[scancode] : kbdus_norm[scancode];
    if (!key) {
        return;
    }

    // Apply Caps Lock to letters
    if (caps_flag) {
        if (key >= 'a' && key <= 'z')
            key = key - ('a' - 'A');
        else if (key >= 'A' && key <= 'Z')
            key = key + ('a' - 'A');
    }

    // Custom input logic: Enter, Backspace, or normal char
    if (key == '\n') {
        // On first Enter, capture buffer (no full-line echo)
        if (gets_flag == 0) {
            KLOG(LOG_LEVEL_DEBUG, "Enter key pressed, capturing input buffer\n");
            do_gets();
        }
        gets_flag++;
        // Clear raw buffer for next line
        while (kb_count > 0)
            buffer[--kb_count] = 0;
        // Echo single newline so shell output appears cleanly
        put('\n');
    }
    else if (key == '\b') {
        if (kb_count > 0) {
            buffer[--kb_count] = 0;
            // move cursor back, overwrite with space, move back again
            put('\b');
            put(' ');
            put('\b');
            KLOG(LOG_LEVEL_DEBUG, "Backspace: removed one character, new count = %d\n", kb_count);
        }
    }
    else {
        if (kb_count < KB_BUFFER_SIZE - 1) {
            buffer[kb_count++] = key;
            put(key);  // echo the character
            KLOG(LOG_LEVEL_DEBUG, "Key pressed: '%c' (scancode=0x%02x), count=%d\n",
                 key, scancode, kb_count);
        }
    }
}

// Initialize keyboard driver
static int init_keyboard(void) {
    register_interrupt_handler(IRQ1, &keyboard_handler);
    KLOG(LOG_LEVEL_INFO, "Keyboard driver initialized\n");
    return 0;
}

// Low-level character read
unsigned char getch(void) {
    unsigned char sc, c;

    // Flush any pending data
    if (kbdus_norm[inb(0x60)] != 0)
        outb(0x60, 0xF4);

    // Wait for key down event
    do {
        sc = inb(0x60);
    } while ((sc & 0x80) || kbdus_norm[sc] == 0);

    // Decode with modifiers
    c = shift_flag ? kbdus_shift[sc] : kbdus_norm[sc];
    outb(0x60, 0xF4);  // acknowledge
    return c;
}

/**
 * Reads a line of input from the keyboard until Enter is pressed.
 * Returns pointer to the static buffer2 (NUL-terminated).
 */
char* gets(void) {
    gets_flag = 0;
    // Busy-wait until handler sets gets_flag
    while (gets_flag == 0) { /* wait */ }
    return (char*)buffer2;
}

/**
 * Returns the last captured line (NUL-terminated) after Enter.
 */
char* get_input_buffer(void) {
    return (char*)buffer2;
}

// Register this driver with the kernel
REGISTER_DRIVER(init_keyboard, DRV_KEYBOARD);

