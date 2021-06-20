#pragma once

#include <stdint.h>
#include "string.h"
#include "assert.h"

#ifdef __GNUC__
#define _target_clones_sse_avx_avx512 \
    __attribute__((__target_clones__("default,avx,avx512f")))
#endif

float trunc(float f);

float fmodf(float x, float y);

// https://web.archive.org/web/20200628195036/http://mooooo.ooo/chebyshev-sine-approximation/
static constexpr float const sin_coeffs[] = {
    -0.10132118f,          // x
     0.0066208798f,        // x^3
    -0.00017350505f,       // x^5
     0.0000025222919f,     // x^7
    -0.000000023317787f,   // x^9
     0.00000000013291342f, // x^11
};


// sin approximation to 5 ulp with Chebyshev polynomials
float sinf(float x);

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
static inline float recip_sqrt_fast(float n)
{
    uint32_t i;
    float x2, y;
    const float threehalves = 1.5F;
    
    x2 = n * 0.5F;
    y  = n;
    memcpy(&i, &y, sizeof(i));
    // evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 );
    memcpy(&y, &i, sizeof(y));
    
    // 1st iteration
    y  = y * ( threehalves - ( x2 * y * y ) );   
    
    // 2nd iteration, this can be removed
    //y  = y * ( threehalves - ( x2 * y * y ) );   
    
    return y;
}

#if defined(__x86_64__) || defined(__i386__)
static inline void sincosf(
    float a, float * __restrict s, float * __restrict c)
{
    __asm__(
        "fsincos\n\t"
        : [c] "+t" (a)
        , [s] "=u" (*s)
    );
    *c = a;
}

static inline void sincos(double a, 
    double * __restrict s, double * __restrict c)
{
    __asm__(
        "fsincos\n\t"
        : [c] "+t" (a)
        , [s] "=u" (*s)
    );
    *c = a;
}

static inline float sqrtf(float n)
{
    float result;
#if defined(__x86_64__s)
    __asm__("sqrtss %[src],%[dst]" 
        : [dst] "=x" (result) 
        : [src] "x" (n)
    );
#else
    result = n;
    __asm__("fsqrt\n\t" 
        : [dst] "+t" (result)
    );
#endif
    return result;
}

static inline float rsqrtf(float n)
{
    float result;
    __asm__("rsqrtss %[src],%[dst]" 
        : [dst] "=x" (result) 
        : [src] "x" (n)
    );
    return result;
}

#else
#error Do not know how to sincos on this CPU yet
#endif
