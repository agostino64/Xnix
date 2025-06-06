#ifndef SERIAL_H
#define SERIAL_H

int serial_init(void);
void serial_write_char(char c);
void serial_write_string(const char* str);

// print message in serial
void printk_serial(const char* fmt, ...);

#endif

