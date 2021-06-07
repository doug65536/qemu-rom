#include "arch/halt.h"

void arch_halt()
{
    while (1)
        __asm__("wfi");
}
