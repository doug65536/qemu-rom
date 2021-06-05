#include "portio.h"
#include "arch/halt.h"
#include "debug.h"

void arch_debug_char(uint8_t ch)
{
    outb(0xe9, ch);
}

void arch_halt()
{
    while (true)
        __asm__("hlt");
}
