#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);
void memset(uint8_t *dest, uint8_t val, uint32_t len);
unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);

int strcmp(char *str1, char *str2);
int strncmp(char s1,char s2,unsigned long n);
char *strcpy(char *dest, const char *src);
void strcat(char *dest, const char *src);
void strncat(char *dest, const char *src, unsigned long n);
int strlen(char *src);
char *strchr (const char* s,int c);
int strspn(const char *s, const char *accept);
char *strpbrk(const char *s, const char *accept);
char *strtok(char *str, const char *delim);

#endif
