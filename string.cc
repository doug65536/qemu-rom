#include "string.h"

void *memcpy(void * __restrict dest, void const * __restrict src, size_t size)
{
    char volatile *d_end = (char volatile *)dest + size;
    char volatile *s_end = (char volatile *)src + size;
    for (size = -size; size != 0; ++size)
        d_end[size] = s_end[size];
    return dest;
}

void *memset(void * __restrict dest, int value, size_t size)
{
    char volatile *out = (char volatile *)dest;
    
    for (size_t i = 0; i < size; ++i)
        out[i] = value & 0xFF;
    
    return (void*)out;
}
