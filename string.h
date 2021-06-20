#pragma once
#include <stddef.h>
#include "compiler.h"

__BEGIN_DECLS

_access(__write_only__, 1, 3) _access(__read_only__, 2, 3)
void *memcpy(void * __restrict dest, void const * __restrict src, size_t size);

_access(__write_only__, 1, 3)
void *memset(void * __restrict dest, int value, size_t size);

size_t strlen(char const *s);

__END_DECLS
