#include "debug.h"

static char volatile *pl011_base = (char volatile *)0x09000000;// len=0x1000

void arch_debug_char(uint8_t ch)
{
    *pl011_base = ch;
}
