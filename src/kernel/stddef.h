#ifndef _STDDEF_H
#define _STDDEF_H

typedef unsigned long size_t;
typedef long ptrdiff_t;
typedef unsigned short wchar_t;

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#endif
