#include "arch/halt.h"

void arch_halt()
{
    while (true)
        __asm__("hlt");
}
