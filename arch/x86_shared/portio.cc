#include "portio.h"

void arch_debug_char(uint8_t ch)
{
    outb(0xe9, ch);
}

extern "C" void arch_halt()
{
    while (true)
        __asm__("hlt");
}
