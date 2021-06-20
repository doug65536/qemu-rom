#pragma once

#ifdef __GNUG__

#if __GNUC__ >= 11
#define _access(...) __attribute__((__access__(__VA_ARGS__)))
#else
#define _access(...)
#endif

#define _noreturn __attribute__((__noreturn__))
#else
#define _access(...)
#endif

#define _constructor(prio) __attribute__((__constructor__(prio)))

#ifdef __cplusplus

#define restrict __restrict
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }

#define __BEGIN_ANONYMOUS        namespace {
#define __BEGIN_NAMESPACE(n)        namespace n {
#define __BEGIN_NAMESPACE_DETAIL    __BEGIN_NAMESPACE(detail)
#define __BEGIN_NAMESPACE_STD       __BEGIN_NAMESPACE(std)
#define __BEGIN_NAMESPACE_EXT       __BEGIN_NAMESPACE(ext)
#define __END_NAMESPACE             }
#define __END_ANONYMOUS             __END_NAMESPACE
#define __END_NAMESPACE_STD         __END_NAMESPACE
#define __END_NAMESPACE_EXT         __END_NAMESPACE
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

#define _stdcall                __attribute__((__stdcall__))

#define _packed                 __attribute((__packed__))
#define _const                  __attribute__((__const__))
#define _pure                   __attribute__((__pure__))
#define _malloc                 __attribute__((__malloc__))
#define _alloc_size(...)        __attribute__((__alloc_size__(__VA_ARGS__)))
#define _alloc_align(pi)        __attribute__((__alloc_align__(pi)))
#ifdef __x86_64__
#define _ms_struct              __attribute__((__ms_struct__))
#else
#define _ms_struct
#endif
#define _unused                 __attribute__((__unused__))
#define _use_result             __attribute__((__warn_unused_result__))
#define _leaf                   __attribute__((__leaf__))
#define _aligned(n)             __attribute__((__aligned__(n)))
#define _always_inline          inline __attribute__((__always_inline__))
#define _noreturn               __attribute__((__noreturn__))
#define _used                   __attribute__((__used__))
#define _returns_twice          __attribute__((__returns_twice__))
#define _vector_size(n)         __attribute__((__vector_size__(n)))
#define _noinline               __attribute__((__noinline__))
#define _flatten                __attribute__((__flatten__))
#define _assume_aligned(n)      __attribute__((__assume_aligned__(n)))
#define _printf_format(m,n)     __attribute__((__format__(__printf__, m, n)))
#define _artificial             __attribute__((__artificial__))
#define _no_instrument          __attribute__((__no_instrument_function__))
#define _no_asan                __attribute__((__no_address_safety_analysis__))
#define _no_ubsan               __attribute__((no_sanitize("undefined")))
#define _no_plt                 __attribute__((__noplt__))

#define _constructor(prio)      __attribute__((__constructor__(prio)))
#define _destructor(prio)       __attribute__((__destructor__(prio)))

#define _ifunc_resolver(fn)     __attribute__((__ifunc__(#fn)))
#define _section(name)          __attribute__((__section__(name)))
#define _hot                    __attribute__((__hot__))
#define _cold                   __attribute__((__cold__))

//#define _access(...)            __attribute__((__access__(__VA_ARGS__)))

#define _assume(expr) \
    do { \
        if (!(expr)) \
            __builtin_unreachable(); \
    } while (0)
