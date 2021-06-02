#include "assert.h"

void __assert_failed(const char *expr, const char *file, int line)
{
    asm("0:hlt\r\njmp 0b" : : "D" (expr), "S" (file), "d" (line));
}
