/*
 *  XnixOS
 *
 *  xnix/core/common.c
 */
 
#include <xnix/common.h>
#include <stdint.h>
#include <stddef.h>

// Copy len bytes from src to dest.
void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(uint8_t *dest, uint8_t val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
    const uint8_t* u8Ptr1 = (const uint8_t *)ptr1;
    const uint8_t* u8Ptr2 = (const uint8_t *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}

// Compare two strings. Should return -1 if 
// str1 < str2, 0 if they are equal or 1 otherwise.
int strcmp(char *str1, char *str2)
{
      int i = 0;
      int failed = 0;
      while(str1[i] != '\0' && str2[i] != '\0')
      {
          if(str1[i] != str2[i])
          {
              failed = 1;
              break;
          }
          i++;
      }
      // why did the loop exit?
      if( (str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0') )
          failed = 1;
  
      return failed;
}

int strncmp(char s1,char s2,unsigned long n)
{
  unsigned char u1, u2;

  while (n-- > 0)
    {
      u1 = (unsigned char) s1++;
      u2 = (unsigned char) s2++;
      if (u1 != u2)
	return u1 - u2;
      if (u1 == '\0')
	return 0;
    }
  return 0;
}

char *strcpy(char *dest, const char *src)
{
    while( *src )
	*(dest++) =  *(src++);
    *dest='\0';
    return dest;
}

void strcat(char *dest, const char *src)
{
    int dest_len = strlen(dest);
    while (*src != '\0') {
        dest[dest_len++] = *src++;
    }
    dest[dest_len] = '\0';
}

void strncat(char *dest, const char *src, unsigned long n)
{
    int dest_len = strlen(dest);
    int i;
    for (i = 0; i < n && src[i] != '\0'; ++i)
        dest[dest_len + i] = src[i];
    dest[dest_len + i] = '\0';
}

unsigned strlen(const char* str)
{
    unsigned len = 0;
    while (*str)
    {
        ++len;
        ++str;
    }

    return len;
}

char *strchr(const char* s,int c)
{
  do
  {
    if (*s == c)
	return (char*)s;
  }
  while (*s++);
  return (0);
}

int strspn(const char *s, const char *accept)
{
    int count = 0;
    const char* p;
    while (*s != '\0')
    {
        for (p = accept; *p != '\0'; ++p)
        {
            if (*s == *p)
                break;
        }
        if (*p == '\0')
            return count;
        ++count;
        ++s;
    }
    return count;
}

char *strpbrk(const char *s, const char *accept)
{
    const char* p;
    while (*s != '\0')
    {
        for (p = accept; *p != '\0'; ++p)
        {
            if (*s == *p)
                return (char*)s;
        }
        ++s;
    }
    return NULL;
}

char *strtok(char *str, const char *delim)
{
    static char* last_token = NULL;
    char* token = NULL;

    if (str != NULL)
        last_token = str;
    else
    {
        if (last_token == NULL)
            return NULL;
        str = last_token;
    }

    str += strspn(str, delim);
    if (*str == '\0')
    {
        last_token = NULL;
        return NULL;
    }

    token = str;
    str = strpbrk(token, delim);
    if (str == NULL)
        last_token = NULL;
    else
    {
        *str = '\0';
        last_token = str + 1;
    }

    return token;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}
