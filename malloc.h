#pragma once
#include <stdint.h>
#include <stddef.h>
#include "compiler.h"

__BEGIN_DECLS

_use_result _malloc _alloc_size(1) _leaf
void *malloc(size_t bytes);

_use_result _malloc _alloc_size(1, 2) _leaf
void *calloc(size_t num, size_t size);

void free(void *p);

_use_result _malloc _alloc_align(2) _alloc_size(1) _leaf
void *malloc_aligned(size_t bytes, size_t alignment);

_use_result _leaf _alloc_size(2)
void *realloc(void *p, size_t bytes);

_use_result _alloc_size(2) _alloc_align(3)
void *realloc_aligned(void *p, size_t bytes, size_t alignment);

bool malloc_validate();
bool malloc_validate_or_panic();

void malloc_init(void *st, void *en);

void malloc_get_heap_range(void **st, void **en);

char *strdup(char const *s);

#ifndef NDEBUG
void test_malloc();
#endif

__END_DECLS

_malloc _alloc_size(1)
void *operator new(size_t size);

_const
void *operator new(size_t size, void *p);

_malloc _alloc_size(1) _assume_aligned(16)
void *operator new[](size_t size);
void operator delete(void *block, unsigned long size);
void operator delete(void *block) noexcept;
void operator delete[](void *block) noexcept;
void operator delete[](void *block, unsigned int);


__BEGIN_NAMESPACE_EXT

struct nothrow_t {
    explicit nothrow_t() = default;
};
enum class align_val_t : size_t {};
extern nothrow_t const nothrow;

__END_NAMESPACE_EXT

void* operator new[](size_t count, ext::align_val_t alignment);
void* operator new[](size_t count, ext::align_val_t alignment,
    ext::nothrow_t const&) noexcept;
void *operator new(size_t size, ext::nothrow_t const&) noexcept;
void *operator new[](size_t size, ext::nothrow_t const&) noexcept;

void operator delete[](void *block, unsigned long);
void operator delete(void *block, unsigned size);
