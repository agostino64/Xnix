/*
 *  XnixOS
 *
 *  xnix/drivers/keyb.c
 */

#include <xnix/common.h>
#include <xnix/isr.h>
#include <xnix/vga.h>

extern u32 cursor_y, cursor_x;

volatile int shift_flag=0;
volatile int caps_flag=0;

volatile char* buffer; //For storing strings
volatile char* buffer2;
volatile int kb_count = 0; //Position in buffer
volatile int gets_flag = 0;

unsigned short ltmp;
int ktmp = 0;

static void do_gets(void);

//US keymap
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    1,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   1,				/* Right shift */
  '*',
    1,	/* Alt */
  ' ',	/* Space bar */
    1,	/* Caps lock */
    1,	/* 59 - F1 key ... > */
    1,   1,   1,   1,   1,   1,   1,   1,
    1,	/* < ... F11 */
    1,	/* 69 - Num lock*/
    1,	/* Scroll Lock */
    1,	/* Home key */
    1,	/* Up Arrow */
    1,	/* Page Up */
  '-',
    1,	/* Left Arrow */
    1,
    1,	/* Right Arrow */
  '+',
    1,	/* 79 - End key*/
    1,	/* Down Arrow */
    1,	/* Page Down */
    1,	/* Insert Key */
    1,	/* Delete Key */
    1,   1,   '\\',
    1,	/* F11 Key */
    1,	/* F12 Key */
    1,	/* All other keys are undefined */
};

/* Handles the keyboard interrupt */
static void keyboard_handler(registers_t regs)
{
    unsigned char scancode;

    //Read scancode
    scancode = inb(0x60);
    
    switch (scancode)
    {
           case 0x3A:
                /* CAPS_LOCK LEDS */
                outb(0x60,0xED);
                ltmp |= 4;
                outb(0x60,ltmp);
                
                if(caps_flag)
                caps_flag=0;
                else
                caps_flag=1;
                break;
           case 0x45:
                /* NUM_LOCK LEDS */
                outb(0x60,0xED);
                ltmp |= 2;
                outb(0x60,ltmp);
                break;
           case 0x46:
                /* SCROLL_LOCK LEDS */
                outb(0x60,0xED);
                ltmp |= 1;
                outb(0x60,ltmp);
                break;
           case 60: /* F12 */
                //reboot();
                break;
           default:
                break;
    }

    if (scancode & 0x80)
    {
        //Key release
        
        //Left and right shifts
        if (scancode - 0x80 == 42 || scancode - 0x80 == 54)
			shift_flag = 0;
    }
    else
    {   
        //Keypress (normal)
        
        //Shift
        if (scancode == 42 || scancode == 54)
		{
			shift_flag = 1;
			return;
		}
        
        //Gets()
        if(kbdus[scancode] == '\n')
        { 
             // add new line in keyboard buffer
             if(gets_flag == 0) do_gets();
             gets_flag++;
             for(;kb_count; kb_count--)
                  buffer[kb_count] = 0;              
        }
        else 
        {
             // Delete key
             if(kbdus[scancode] == '\b')
             {    
                  if(kb_count)
                    buffer[kb_count--] = 0;
             }
             else
                  buffer[kb_count++] = kbdus[scancode];
        }   
        put(kbdus[scancode]);
        return;
    }
}

void init_keyboard(void)
{
    register_interrupt_handler(IRQ1, &keyboard_handler);
}

//Gets a key
unsigned char getch(void)
{
     unsigned char getch_char;
     
     if(kbdus[inb(0x60)] != 0) //Not empty
     outb(0x60,0xf4); //Clear buffer
     
     while(kbdus[inb(0x60)] == 0); //While buffer is empty
     getch_char = kbdus[inb(0x60)];
     outb(0x60,0xf4); //Leave it emptying
     return getch_char;
}

char* gets(void)
{ 
     gets_flag = 0;
     while(gets_flag == 0);
     return (char*)buffer2;
}

static void do_gets(void)
{
     buffer[kb_count++] = 0; //Null terminated biatch!
     for(;kb_count; kb_count--)
     {
          buffer2[kb_count] = buffer[kb_count];
     }
     return;
}
