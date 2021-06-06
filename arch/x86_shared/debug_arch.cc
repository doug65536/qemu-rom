#include "debug.h"
#include "portio_arch.h"

void arch_debug_char(uint8_t ch)
{
    outb(0xe9, ch);
}
