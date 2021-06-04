#include "assert.h"
#include "debug.h"
#include "arch/halt.h"

void __assert_failed(const char *expr, const char *file, int line)
{
    printdbg("%s(%d): Assertion failed: %s\n", file, line, expr);
    arch_halt();
}
