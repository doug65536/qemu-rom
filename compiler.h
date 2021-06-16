
#ifdef __GNUG__

#if __GNUC__ >= 11
#define _access(...) __attribute__((__access__(__VA_ARGS__)))
#else
#define _access(...)
#endif

#define _noreturn __attribute__((__noreturn__))
#define _always_inline inline __attribute__((__always_inline__))
#define _printf_format(...) __attribute__((__format__(__printf__, __VA_ARGS__)))
#else
#define _noreturn
#define _access(...)
#define _always_inline inline
#define _printf_format(...)
#endif

#define _constructor(prio) __attribute__((__constructor__(prio)))
