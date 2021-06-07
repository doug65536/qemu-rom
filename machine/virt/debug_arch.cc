#include <stdint.h>
#include "debug.h"
#include "build.config.h"

static uint8_t volatile * const uart_base =
        (uint8_t volatile *)SERIAL_DEBUGCON_ST;

void arch_debug_char(uint8_t ch)
{
    *uart_base = ch;
}
