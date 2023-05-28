#ifndef LIBSTR_H
#define LIBSTR_H

#include "stddef.h"

size_t k_strlen(const char *str);
void k_strcpy(char *dest, const char *src);
void k_strncpy(char *dest, const char *src, size_t n);
void k_strcat(char *dest, const char *src);
void k_strncat(char *dest, const char *src, size_t n);
int k_strcmp(const char *s1, const char *s2);
void *k_memcpy(void *dest, const void *src, size_t n);
void k_memset(void *ptr, int value, size_t num);
char *k_strchr(const char *s, int c);
size_t k_strspn(const char *s, const char *accept);
char *k_strpbrk(const char *s, const char *accept);
char *k_strtok(char *str, const char *delim);

#endif

