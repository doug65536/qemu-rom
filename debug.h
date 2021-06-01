#pragma once
#include <stdint.h>
#include <stdarg.h>

extern char const hexlookup[];

__attribute__((__access__(__write_only__, 1, 3)))
__attribute__((__access__(__read_write__, 2)))
__attribute__((__format__(__printf__, 6, 0)))
intptr_t little_formatterv(
        char *buffer, char **buffer_ptr, size_t buffer_size, 
        void (*buffer_char)(char *, char **, size_t, void *, int),
        void *buffer_char_arg,
        char const *format, 
        va_list ap);

__attribute__((__format__(__printf__, 1, 0)))
intptr_t vprintdbg(char const *format, va_list ap);

__attribute__((__format__(__printf__, 1, 2)))
intptr_t printdbg(char const *format, ...);

int putsdbg(char const *s);

