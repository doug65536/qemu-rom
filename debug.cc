#include <stddef.h>
#include "debug.h"
#include "portio.h"

void little_formatterv_default(char *buffer, char **buffer_ptr, 
        size_t buffer_size, void *, int ch)
{
    if (ch >= 0) {
        *(*buffer_ptr)++ = ch;
        *(*buffer_ptr) = 0;
    }
    
    size_t used = *buffer_ptr - buffer;
    if (used + 1 >= buffer_size || ch == '\n' || ch == '\r' || ch == -1) {
        char *end = *buffer_ptr;
        
        // Reset buffer pointer
        *buffer_ptr = buffer;
        
        // Write the buffer to output
        while (buffer < end)
            outb(0xe9, *buffer++);
        
    }
}

char const hexlookup[] = "0123456789abcdef";

intptr_t little_formatterv(
        char *buffer, char **buffer_ptr, size_t buffer_size,
        void (*buffer_char)(char *, char **, size_t, void *, int), 
        void *buffer_char_arg,
        char const *format, va_list ap)
{
    intptr_t total = 0;
    
    char const *p;
    for (p = format; *p; ++p) {
        int ch = uint8_t(*p);
        
        if (ch != '%') {
            ++total;
            buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg, ch);
            continue;
        }

        char const *s;
        bool is_long = false;
        bool is_long_long = false;
        bool is_signed = false;
        uintmax_t unum;
        intmax_t snum;
        int base = -1;
        int width = 0;
        int leading_zeros = 0;
        
        if (*p == '0')
            leading_zeros = 1;
        
        while (*p >= '0' && *p <= '9')
            width = width * 10 + (*p++ - '0');
        
more_format_chars:
        ch = *++p;
        switch (ch) {
        case 's':
            s = va_arg(ap, char const *);

            while (*s) {
                ++total;
                buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg, 
                        uint8_t(*s++));
            }

            break;

        case 'c':
            ch = va_arg(ap, int);

            ++total;
            buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg, ch);

            break;

        case 'l':
            if (!is_long)
                is_long = true;
            else
                is_long_long = true;

            goto more_format_chars;
        
        case 'z':
            is_long = true;
            
            goto more_format_chars;
                    
        case 'd':
            if (is_long_long)
                snum = va_arg(ap, long long);
            else if (is_long)
                snum = va_arg(ap, long);
            else
                snum = va_arg(ap, int);

            is_signed = true;
            base = 10;

            break;

        case 'o':
        case 'u':
        case 'x':
            if (is_long_long)
                unum = va_arg(ap, unsigned long long);
            else if (is_long)
                unum = va_arg(ap, unsigned long);
            else
                unum = va_arg(ap, unsigned);

            base = ch == 'x' ? 16 : ch == 'u' ? 10 : 8;

            break;

        case 'p':
            unum = (uintmax_t)va_arg(ap, void *);

            base = 16;

            break;

        }

        if (base < 0)
            continue;

        char number[32];
        char * const number_end = number + sizeof(number);
        char *digit = number_end;
        
        bool is_negative = false;

        if (is_signed) {
            is_negative = snum < 0;
            
            if (is_negative)
                snum = -snum;
            
            do {
                *--digit = hexlookup[snum % base];
                snum /= base;
            } while (snum);
            
            if (leading_zeros) {
                while (digit > number && number_end - digit < width)
                    *--digit = '0';
            }
            
            while (digit > number && number_end - digit < width)
                    *--digit = ' ';
        } else {
            do {
                *--digit = hexlookup[unum % base];
                unum /= base;
            } while (unum);
        }
        
        int digit_len = number_end - digit;
        int leading_char_count = width - digit_len;
        
        if (is_negative)
            --leading_char_count;
        
        char leading_char;
        
        if (leading_zeros) {
            leading_char = '0';
            
            if (is_negative) {
                // Minus symbol goes before the leading zeros
                buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg,
                        '-');
            }
        } else {
            leading_char = ' ';
        }
        
        while (leading_char_count > 0) {
            --leading_char_count;
            buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg,
                        leading_char);
        }
        
        if (!leading_zeros && is_negative)
            buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg,
                    '-');
        
        while (digit < number_end) {
            ++total;
            buffer_char(buffer, buffer_ptr, buffer_size, buffer_char_arg,
                    uint8_t(*digit++));
        }
    }
    
    return total;
}

intptr_t vprintdbg(char const *format, va_list ap)
{
    char buffer[82];
    char *ptr = buffer;
    intptr_t result = little_formatterv(buffer, &ptr, sizeof(buffer),
            little_formatterv_default, nullptr, format, ap);
    little_formatterv_default(buffer, &ptr, sizeof(buffer), nullptr, -1);
    return result;
}

intptr_t printdbg(char const *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int result = vprintdbg(format, ap);
    va_end(ap);
    return result;
}

int putsdbg(char const *s)
{
    char buffer[82];
    char *ptr = buffer;
    int total = 0;
    while (*s) {
        ++total;
        little_formatterv_default(buffer, &ptr, sizeof(buffer), 
                nullptr, uint8_t(*s));
    }
    little_formatterv_default(buffer, &ptr, sizeof(buffer), nullptr, -1);
    
    return total;
}
