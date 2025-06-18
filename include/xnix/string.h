#ifndef STRING_H
#define STRING_H

#include "common.h"

const char* strchr(const char* str, char chr);
char* strcpy(char* dst, const char* src);
unsigned strlen(const char* str);
int strcmp(const char* a, const char* b);

#endif