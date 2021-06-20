#pragma once

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x)   __builtin_expect(!!(x), 0)

#ifndef __assume_defined
#define __assume_defined
#define assume(x) (likely((x)) ? 1 : (__builtin_unreachable(), 0))
#endif
