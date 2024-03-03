// common.c -- Defines some global functions.
// From JamesM's kernel development tutorials.

#include <xnix/common.h>

// Write a byte out to the specified port.
void outb(u16 port, u8 value)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(u16 port, u16 value)
{
    __asm__ __volatile__ ( "outw %w0, %1" : : "a" (value), "id" (port) );
}

u8 inb(u16 port)
{
   u8 ret;
   __asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

u16 inw(u16 port)
{
   u16 ret;
   __asm__ __volatile__ ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

// Write len copies of val into dest.
void memset(void *dest, u8 val, u32 len)
{
    for (u8 *temp = dest; len != 0; len--)
      *temp++ = val;
}

// Copy len bytes from src to dest.
void memcpy(u8 *dest, const u8 *src, u32 len)
{
    const u8 *sp = (const u8 *)src;
    u8 *dp = (u8 *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

// Compare two strings. Should return -1 if 
// str1 < str2, 0 if they are equal or 1 otherwise.
int strcmp(char *str1, char *str2)
{
      int i = 0;
      while(str1[i] != '\0' && str2[i] != '\0')
      {
          if(str1[i] != str2[i])
          {
              return 1;
              break;
          }
          i++;
      }
      // why did the loop exit?
      if( (str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0') )
          return -1;
  
      return 0;
}
