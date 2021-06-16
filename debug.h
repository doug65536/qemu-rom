#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "compiler.h"

extern char const hexlookup[];

extern "C" 
_access(__write_only__, 1, 3)
_access(__read_write__, 2)
_printf_format(6, 0)
intptr_t little_formatterv(
        char *buffer, char **buffer_ptr, size_t buffer_size, 
        void (*buffer_char)(char *, char **, size_t, void *, int),
        void *buffer_char_arg,
        char const *format, 
        va_list ap);

void little_formatterv_default(char *buffer, char **buffer_ptr, 
        size_t buffer_size, void *, int ch);

extern "C" 
_printf_format(1, 0)
intptr_t vprintdbg(char const *format, va_list ap);

extern "C" 
_printf_format(1, 2)
intptr_t printdbg(char const *format, ...);

extern "C" 
int putsdbg(char const *s);

extern "C" 
void arch_debug_char(uint8_t ch);
