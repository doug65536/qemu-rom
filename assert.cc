#include "assert.h"
#include "debug.h"
#include "arch/halt.h"

static int assert_nest;

void __assert_failed(const char *expr, const char *file, int line)
{
    int nesting = __atomic_fetch_add(&assert_nest, 1, __ATOMIC_RELAXED);
    if (!nesting)
        printdbg("%s(%d): Assertion failed: %s\n", file, line, expr);
    arch_halt();
}
