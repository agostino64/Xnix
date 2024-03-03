#include <xnix/common.h>

#define FOREGROUND 0  // Black
#define BACKGROUND 7  // Grey

#define TAB_SIZE 4

u16 *video_memory = (u16*)0xb8000;
u32 cursor_y, cursor_x;

// Updates the hardware cursor.
static void move_cursor(void)
{
   // The screen is 80 characters wide...
   u16 cursorLocation = cursor_y * 80 + cursor_x;
    
   outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
   outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
   outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
   outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void scroll(void)
{

   // Get a space character with the default colour attributes.
   u8 attributeByte = (FOREGROUND << 4) | (BACKGROUND & 0x0F);
   u16 blank = 0x20 /* space */ | (attributeByte << 8);

   // Row 25 is the end, this means we need to scroll up
   if(cursor_y >= 25)
   {
       // Move the current text chunk that makes up the screen
       // back in the buffer by a line
       s32 i;
       for (i = 0*80; i < 24*80; i++)
       {
           video_memory[i] = video_memory[i+80];
       }

       // The last line should now be blank. Do this by writing
       // 80 spaces to it.
       for (i = 24*80; i < 25*80; i++)
       {
           video_memory[i] = blank;
       }
       // The cursor should now be on the last line.
       cursor_y = 24;
   }
} 

// Writes a single character out to the screen.
void put(const char c)
{
   // The attribute byte is made up of two nibbles - the lower being the
   // foreground colour, and the upper the background colour.
   u8  attributeByte = (FOREGROUND << 4) | (BACKGROUND & 0x0F);
   // The attribute byte is the top 8 bits of the word we have to send to the
   // VGA board.
   u16 attribute = attributeByte << 8;
   u16 *location;

   // Handle a backspace, by moving the cursor back one space
   if (c == '\b')
   {   
       if(cursor_x != 0 && cursor_x != 3) cursor_x--;
       put(' ');
       if(cursor_x != 0 && cursor_x != 3) cursor_x--;
   }

   // Handle a tab by increasing the cursor's X, but only to a point
   // where it is divisible by 8.
   if (c == '\t')
       cursor_x = cursor_x + TAB_SIZE;
   
   // Handle carriage return
   if (c == '\r')
       cursor_x = 0;

   // Handle newline by moving cursor back to left and increasing the row
   if (c == '\n')
   {
       cursor_x = 0;
       cursor_y++;
   }
   // Handle any other printable character.
   if(c >= ' ')
   {
       location = video_memory + (cursor_y*80 + cursor_x);
       *location = c | attribute;
       cursor_x++;
   }

   // Check if we need to insert a new line because we have reached the end
   // of the screen.
   if (cursor_x >= 80)
   {
       cursor_x = 0;
       cursor_y ++;
   }

   // Scroll the screen if needed.
   scroll();
   // Move the hardware cursor.
   move_cursor();
} 

// Clears the screen, by copying lots of spaces to the framebuffer.
void clear_screen(void)
{
   // Make an attribute byte for the default colours
   u8 attributeByte = (FOREGROUND << 4) | (BACKGROUND & 0x0F);
   u16 blank = 0x20 /* space */ | (attributeByte << 8);

   s32 i;
   for (i = 0; i < 80*25; i++)
   {
       video_memory[i] = blank;
   }

   // Move the hardware cursor back to the start.
   cursor_x = 0;
   cursor_y = 0;
   move_cursor();
}

// Outputs a null-terminated ASCII string to the monitor.
void write(const char *c)
{
   s32 i = 0;
   while (c[i])
   {
       put(c[i++]);
   }
}

#define PRINTK_STATE_NORMAL         0
#define PRINTK_STATE_LENGTH         1
#define PRINTK_STATE_LENGTH_SHORT   2
#define PRINTK_STATE_LENGTH_LONG    3
#define PRINTK_STATE_SPEC           4

#define PRINTK_LENGTH_DEFAULT       0
#define PRINTK_LENGTH_SHORT_SHORT   1
#define PRINTK_LENGTH_SHORT         2
#define PRINTK_LENGTH_LONG          3

const char hexchars[] = "0123456789abcdef";

void printk_unsigned(unsigned long number, s32 radix)
{
    char buffer[32];
    s32 pos = 0;

    // convert number to ASCII
    do 
    {
        unsigned long rem = number % radix;
        number /= radix;
        buffer[pos++] = hexchars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        put(buffer[pos]);
}

void printk_signed(long number, s32 radix)
{
    if (number < 0)
    {
        put('-');
        printk_unsigned(-number, radix);
    }
    else printk_unsigned(number, radix);
}

void vprintk(const char* fmt, va_list args)
{
    s32 state = PRINTK_STATE_NORMAL;
    s32 length = PRINTK_LENGTH_DEFAULT;
    s32 radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTK_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%':   state = PRINTK_STATE_LENGTH;
                                break;
                    default:    put(*fmt);
                                break;
                }
                break;

            case PRINTK_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = PRINTK_LENGTH_SHORT;
                                state = PRINTK_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTK_LENGTH_LONG;
                                state = PRINTK_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTK_STATE_SPEC_;
                }
                break;

            case PRINTK_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = PRINTK_LENGTH_SHORT_SHORT;
                    state = PRINTK_STATE_SPEC;
                }
                else goto PRINTK_STATE_SPEC_;
                break;

            case PRINTK_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = PRINTK_LENGTH_LONG;
                    state = PRINTK_STATE_SPEC;
                }
                else goto PRINTK_STATE_SPEC_;
                break;

            case PRINTK_STATE_SPEC:
            PRINTK_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c':   put((char)va_arg(args, s32));
                                break;

                    case 's':   
                                write((char*)va_arg(args, const char*));
                                break;

                    case '%':   put('%');
                                break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true;
                                break;

                    case 'u':   radix = 10; sign = false; number = true;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false; number = true;
                                break;

                    case 'o':   radix = 8; sign = false; number = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }

                if (number)
                {
                    if (sign)
                    {
                        switch (length)
                        {
                        case PRINTK_LENGTH_SHORT_SHORT:
                        case PRINTK_LENGTH_SHORT:
                        case PRINTK_LENGTH_DEFAULT:     printk_signed(va_arg(args, s32), radix);
                                                        break;

                        case PRINTK_LENGTH_LONG:        printk_signed(va_arg(args, long), radix);
                                                        break;
                        }
                    }
                    else
                    {
                        switch (length)
                        {
                        case PRINTK_LENGTH_SHORT_SHORT:
                        case PRINTK_LENGTH_SHORT:
                        case PRINTK_LENGTH_DEFAULT:     printk_unsigned(va_arg(args, u32), radix);
                                                        break;
                                                        
                        case PRINTK_LENGTH_LONG:        printk_unsigned(va_arg(args, unsigned long), radix);
                                                        break;


                        }
                    }
                }

                // reset state
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

void printk(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
