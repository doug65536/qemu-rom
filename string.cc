#include "string.h"

void *memcpy(void * __restrict dest, void const * __restrict src, size_t size)
{
    char *d_end = (char*)dest + size;
    char *s_end = (char*)src + size;
    for (size = -size; size != 0; ++size)
        d_end[size] = s_end[size];
    return dest;
}
