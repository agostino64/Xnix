#ifndef ASM_H
#define ASM_H

 void cli();     /* Disable ints */
 void sti();     /* Enable ints */
 void halt();    /* Halts CPU */
 void reboot();  /* Reboots CPU */
 
 unsigned char readCMOS(unsigned char addr);             /* Get CMOS data */
 void writeCMOS(unsigned char addr, unsigned int value); /* Write CMOS data */
 
#endif
