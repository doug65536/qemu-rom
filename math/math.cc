
float trunc(float f)
{
    uint32_t n;
    memcpy(&n, &f, sizeof(n));
    
    int exponent = ((n >> 23) & 0xff) - 0x7f;
    
    // See if we keep none of the bits (zero out negative)
    if (exponent < 0)
        exponent = 0;
    
    // See if we keep all the bits
    if (exponent >= 23)
        return f;
    
    uint32_t bits_kept = exponent >= 0 ? exponent : 0;
    
    uint32_t mask = -(1U << (23 - bits_kept));
    
    n &= mask;
    
    float r;
    memcpy(&r, &n, sizeof(r));
    
    return r;
}

float fmodf(float x, float y) 
{
    return x - trunc(x / y) * y;
}

float sinf(float x)
{
    float pi_major = 3.1415927f;
    //    x = fmodf(x, pi_major);
    float pi_minor = -0.00000008742278f;
    float x2 = x*x;
    float p11 = sin_coeffs[5];
    float p9  = p11*x2 + sin_coeffs[4];
    float p7  = p9*x2  + sin_coeffs[3];
    float p5  = p7*x2  + sin_coeffs[2];
    float p3  = p5*x2  + sin_coeffs[1];
    float p1  = p3*x2  + sin_coeffs[0];
    return (x - pi_major - pi_minor) *
            (x + pi_major + pi_minor) * p1 * x;
}
