#ifndef _STDARG_H
#define _STDARG_H

typedef char* va_list;

#define va_start(ap, last_arg) ((void)((ap) = (va_list)&(last_arg) + sizeof(last_arg)))
#define va_arg(ap, type) (*(type*)((ap) += sizeof(type), (ap) - sizeof(type)))
#define va_end(ap) ((void)(ap = (va_list)0))

#endif /* _STDARG_H */
