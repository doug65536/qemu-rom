#include "portio_arch.h"
#include "arch/halt.h"
#include "debug.h"

void arch_halt()
{
    while (true)
        __asm__("hlt");
}
