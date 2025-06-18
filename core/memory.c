#include <xnix/common.h>
#include <xnix/memory.h>


void* memcpy(void* dst, const void* src, u16 num)
{
    u8* u8Dst = (u8 *)dst;
    const u8* u8Src = (const u8 *)src;

    for (u16 i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void * memset(void * ptr, int value, u16 num)
{
    u8* u8Ptr = (u8 *)ptr;

    for (u16 i = 0; i < num; i++)
        u8Ptr[i] = (u8)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, u16 num)
{
    const u8* u8Ptr1 = (const u8 *)ptr1;
    const u8* u8Ptr2 = (const u8 *)ptr2;

    for (u16 i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}