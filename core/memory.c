#include <stdint.h>
#include <xnix/common.h>
#include <xnix/memory.h>


void* memcpy(void* dst, const void* src, uint16_t num)
{
    uint8_t* uint8_tDst = (uint8_t *)dst;
    const uint8_t* uint8_tSrc = (const uint8_t *)src;

    for (uint16_t i = 0; i < num; i++)
        uint8_tDst[i] = uint8_tSrc[i];

    return dst;
}

void * memset(void * ptr, int value, uint16_t num)
{
    uint8_t* uint8_tPtr = (uint8_t *)ptr;

    for (uint16_t i = 0; i < num; i++)
        uint8_tPtr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
    const uint8_t* uint8_tPtr1 = (const uint8_t *)ptr1;
    const uint8_t* uint8_tPtr2 = (const uint8_t *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (uint8_tPtr1[i] != uint8_tPtr2[i])
            return 1;

    return 0;
}
