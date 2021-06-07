#pragma once
#include <stddef.h>
#include "compiler.h"

extern "C"
_access(__write_only__, 1, 3)
_access(__read_only__, 2, 3)
void *memcpy(void * __restrict dest, void const * __restrict src, size_t size);
