#ifndef SERIAL_H
#define SERIAL_H

void serial_init(void);
void serial_write_char(char c);
void serial_write_string(const char* str);

// print message in serial
void printk_serial(const char* fmt, ...);
void printk_serial_timetamp(const char* fmt, ...);

#endif

