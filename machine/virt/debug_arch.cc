#include <stdint.h>
#include "debug.h"
#include "config.h"

static uint8_t volatile * const uart_base =
        (uint8_t volatile *)SERIAL_ST;

void arch_debug_char(uint8_t ch)
{
    *uart_base = ch;
}
