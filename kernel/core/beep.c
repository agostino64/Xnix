/*
 *  XnixOS
 *
 *  xnix/core/beep.c
 */
 
#include <xnix/common.h>
#include <xnix/timer.h>
#include <xnix/io.h>
#include <stdint.h>

//Play sound using built in speaker
static void play_sound(int nFrequence)
{
     uint32_t Div;
     uint8_t tmp;

     Div = 1193180 / nFrequence;
     outb(0x43, 0xb6);
     outb(0x42, (uint8_t) (Div) );
     outb(0x42, (uint8_t) (Div >> 8));
     tmp = inb(0x61);
     if (tmp != (tmp | 3))
	outb(0x61, tmp | 3);
}

//make is shutup
static void nosound(void)
{
     uint8_t tmp = (inb(0x61) & 0xFC);
    
     outb(0x61, tmp);
}

//Make a beep
void beep(void)
{
     play_sound(1000);
     timer_wait(10);
     nosound();
}
