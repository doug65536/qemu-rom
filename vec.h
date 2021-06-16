#pragma once

#include "math/math.h"

struct vec3 {
    float x, y, z, w;
    
    vec3() : x(0), y(0), z(0), w(0) {}

    vec3(float x, float y, float z, float w = 0)
        : x(x), y(y), z(z), w(w) {}
    
    explicit vec3(float n)
        : x(n), y(n), z(n), w(n) {}
    
    vec3 operator+(vec3 const& rhs) const
    {
        return {
            x + rhs.x,
            y + rhs.y,
            z + rhs.z,
            w + rhs.w
        };
    }
   
    vec3 &operator+=(vec3 const& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    
    vec3 operator-(vec3 const& rhs) const
    {
        return {
            x - rhs.x,
            y - rhs.y,
            z - rhs.z,
            w - rhs.w
        };
    }
   
    vec3 &operator-=(vec3 const& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
 
    vec3 operator*(float rhs) const
    {
        return {
            x * rhs,
            y * rhs,
            z * rhs,
            w * rhs
        };
    }

    vec3 &operator*=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }

    vec3 operator/(float rhs) const
    {
        rhs = 1.0f / rhs;
        return {
            x * rhs,
            y * rhs,
            z * rhs,
            w * rhs
        };
    }

    vec3 &operator/=(float rhs)
    {
        rhs = 1.0f / rhs;
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }

    vec3 cross(vec3 const& rhs)
    {
        return {
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x,
            0
        };
    }

    float dot(vec3 const& rhs)
    {
        return x * rhs.x + y * rhs.y + z;
    }
    
    float sq_len() const
    {
        return x * x + y * y + z * z;
    }
   
    float recip_len() const
    {
        return rsqrtf(sq_len());
    }

    vec3 &normalize_fast()
    {
        float rl = recip_len();
        x *= rl;
        y *= rl;
        z *= rl;
        w *= rl;
        return *this;
    }

    vec3 normalized_fast()
    {
        float rl = recip_len();
        return {
            x * rl,
            y * rl,
            z * rl,
            w * rl
        };
    }
};

struct mat4x4 {
    float m[4][4];
    
    constexpr mat4x4()
        : m{
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        }
    {
    }

    constexpr mat4x4(float const (&rhs)[4][4])
        : m{
            { rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3] },
            { rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3] },
            { rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3] },
            { rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3] }
        }
    {
    }

    constexpr mat4x4(float const (&a)[4], float const (&b)[4], 
            float const (&c)[4], float const (&d)[4])
        : m{
            { a[0], a[1], a[2], a[3] },
            { b[0], b[1], b[2], b[3] },
            { c[0], c[1], c[2], c[3] },
            { d[0], d[1], d[2], d[3] }
        }
    {
    }

    mat4x4 mul(mat4x4 const& rhs) const
    {
        return {
            {
                m[0][0]*rhs.m[0][0] + m[0][1]*rhs.m[1][0] + 
                m[0][2]*rhs.m[2][0] + m[0][3]*rhs.m[3][0],
                m[0][0]*rhs.m[0][1] + m[0][1]*rhs.m[1][1] +
                m[0][2]*rhs.m[2][1] + m[0][3]*rhs.m[3][1],
                m[0][0]*rhs.m[0][2] + m[0][1]*rhs.m[1][2] + 
                m[0][2]*rhs.m[2][2] + m[0][3]*rhs.m[3][2],
                m[0][0]*rhs.m[0][3] + m[0][1]*rhs.m[1][3] + 
                m[0][2]*rhs.m[2][3] + m[0][3]*rhs.m[3][3]
            },
            {
                m[1][0]*rhs.m[0][0] + m[1][1]*rhs.m[1][0] +
                m[1][2]*rhs.m[2][0] + m[1][3]*rhs.m[3][0],
                m[1][0]*rhs.m[0][1] + m[1][1]*rhs.m[1][1] + 
                m[1][2]*rhs.m[2][1] + m[1][3]*rhs.m[3][1],
                m[1][0]*rhs.m[0][2] + m[1][1]*rhs.m[1][2] + 
                m[1][2]*rhs.m[2][2] + m[1][3]*rhs.m[3][2],
                m[1][0]*rhs.m[0][3] + m[1][1]*rhs.m[1][3] + 
                m[1][2]*rhs.m[2][3] + m[1][3]*rhs.m[3][3]
            },
            {
                m[2][0]*rhs.m[0][0] + m[2][1]*rhs.m[1][0] + 
                m[2][2]*rhs.m[2][0] + m[2][3]*rhs.m[3][0],
                m[2][0]*rhs.m[0][1] + m[2][1]*rhs.m[1][1] + 
                m[2][2]*rhs.m[2][1] + m[2][3]*rhs.m[3][1],
                m[2][0]*rhs.m[0][2] + m[2][1]*rhs.m[1][2] + 
                m[2][2]*rhs.m[2][2] + m[2][3]*rhs.m[3][2],
                m[2][0]*rhs.m[0][3] + m[2][1]*rhs.m[1][3] + 
                m[2][2]*rhs.m[2][3] + m[2][3]*rhs.m[3][3]
            },
            {
                m[3][0]*rhs.m[0][0] + m[3][1]*rhs.m[1][0] + 
                m[3][2]*rhs.m[2][0] + m[3][3]*rhs.m[3][0],
                m[3][0]*rhs.m[0][1] + m[3][1]*rhs.m[1][1] + 
                m[3][2]*rhs.m[2][1] + m[3][3]*rhs.m[3][1],
                m[3][0]*rhs.m[0][2] + m[3][1]*rhs.m[1][2] + 
                m[3][2]*rhs.m[2][2] + m[3][3]*rhs.m[3][2],
                m[3][0]*rhs.m[0][3] + m[3][1]*rhs.m[1][3] + 
                m[3][2]*rhs.m[2][3] + m[3][3]*rhs.m[3][3]
            }
        };
    }

    mat4x4 operator*(mat4x4 const& rhs) const
    {
        return mul(rhs);
    }
    
    mat4x4 transposed() const
    {
        return {
            { m[0][0], m[1][0], m[2][0], m[3][0] },
            { m[0][1], m[1][1], m[2][1], m[3][1] },
            { m[0][2], m[1][2], m[2][2], m[3][2] },
            { m[0][3], m[1][3], m[2][3], m[3][3] }
        };
    }
    
    float determinant() const noexcept
	{
		// 72 multiplies, 23 adds (before CSE)
		return
              (m[0][3] * m[1][2] * m[2][1] * m[3][0])
            - (m[0][2] * m[1][3] * m[2][1] * m[3][0])
            - (m[0][3] * m[1][1] * m[2][2] * m[3][0])
            + (m[0][1] * m[1][3] * m[2][2] * m[3][0])
            + (m[0][2] * m[1][1] * m[2][3] * m[3][0])
            - (m[0][1] * m[1][2] * m[2][3] * m[3][0])
            - (m[0][3] * m[1][2] * m[2][0] * m[3][1])
            + (m[0][2] * m[1][3] * m[2][0] * m[3][1])
            + (m[0][3] * m[1][0] * m[2][2] * m[3][1])
            - (m[0][0] * m[1][3] * m[2][2] * m[3][1])
            - (m[0][2] * m[1][0] * m[2][3] * m[3][1])
            + (m[0][0] * m[1][2] * m[2][3] * m[3][1])
            + (m[0][3] * m[1][1] * m[2][0] * m[3][2])
            - (m[0][1] * m[1][3] * m[2][0] * m[3][2])
            - (m[0][3] * m[1][0] * m[2][1] * m[3][2])
            + (m[0][0] * m[1][3] * m[2][1] * m[3][2])
            + (m[0][1] * m[1][0] * m[2][3] * m[3][2])
            - (m[0][0] * m[1][1] * m[2][3] * m[3][2])
            - (m[0][2] * m[1][1] * m[2][0] * m[3][3])
            + (m[0][1] * m[1][2] * m[2][0] * m[3][3])
            + (m[0][2] * m[1][0] * m[2][1] * m[3][3])
            - (m[0][0] * m[1][2] * m[2][1] * m[3][3])
            - (m[0][1] * m[1][0] * m[2][2] * m[3][3])
            + (m[0][0] * m[1][1] * m[2][2] * m[3][3]);
	}

	// Compute fully arbitrary inverse matrix
	// (264 multiplies, 103 adds, but there are many common subexpressions)
	mat4x4 inverse() const noexcept
	{
        float det = determinant();
		
		assert(det != 0.0f);
		
		if (det == 0)
			return *this;
        
        det = 1.0f / det;

		// 192 multiplies, 80 adds
		
		return {
            {
                det * (
                      (m[1][2]*m[2][3]*m[3][1])
                    - (m[1][3]*m[2][2]*m[3][1])
                    + (m[1][3]*m[2][1]*m[3][2])
                    - (m[1][1]*m[2][3]*m[3][2])
                    - (m[1][2]*m[2][1]*m[3][3])
                    + (m[1][1]*m[2][2]*m[3][3])
                ),
                det * (
                      (m[0][3]*m[2][2]*m[3][1])
                    - (m[0][2]*m[2][3]*m[3][1])
                    - (m[0][3]*m[2][1]*m[3][2])
                    + (m[0][1]*m[2][3]*m[3][2])
                    + (m[0][2]*m[2][1]*m[3][3])
                    - (m[0][1]*m[2][2]*m[3][3])
                ),
                det * (
                      (m[0][2]*m[1][3]*m[3][1])
                    - (m[0][3]*m[1][2]*m[3][1])
                    + (m[0][3]*m[1][1]*m[3][2])
                    - (m[0][1]*m[1][3]*m[3][2])
                    - (m[0][2]*m[1][1]*m[3][3])
                    + (m[0][1]*m[1][2]*m[3][3])
                ),
                det * (
                      (m[0][3]*m[1][2]*m[2][1])
                    - (m[0][2]*m[1][3]*m[2][1])
                    - (m[0][3]*m[1][1]*m[2][2])
                    + (m[0][1]*m[1][3]*m[2][2])
                    + (m[0][2]*m[1][1]*m[2][3])
                    - (m[0][1]*m[1][2]*m[2][3])
                )
            }, {
                det * (
                      (m[1][3]*m[2][2]*m[3][0])
                    - (m[1][2]*m[2][3]*m[3][0])
                    - (m[1][3]*m[2][0]*m[3][2])
                    + (m[1][0]*m[2][3]*m[3][2])
                    + (m[1][2]*m[2][0]*m[3][3])
                    - (m[1][0]*m[2][2]*m[3][3])
                ),
                det * (
                      (m[0][2]*m[2][3]*m[3][0])
                    - (m[0][3]*m[2][2]*m[3][0])
                    + (m[0][3]*m[2][0]*m[3][2])
                    - (m[0][0]*m[2][3]*m[3][2])
                    - (m[0][2]*m[2][0]*m[3][3])
                    + (m[0][0]*m[2][2]*m[3][3])
                ),
                det * (
                      (m[0][3]*m[1][2]*m[3][0])
                    - (m[0][2]*m[1][3]*m[3][0])
                    - (m[0][3]*m[1][0]*m[3][2])
                    + (m[0][0]*m[1][3]*m[3][2])
                    + (m[0][2]*m[1][0]*m[3][3])
                    - (m[0][0]*m[1][2]*m[3][3])
                ),
                det * (
                      (m[0][2]*m[1][3]*m[2][0])
                    - (m[0][3]*m[1][2]*m[2][0])
                    + (m[0][3]*m[1][0]*m[2][2])
                    - (m[0][0]*m[1][3]*m[2][2])
                    - (m[0][2]*m[1][0]*m[2][3])
                    + (m[0][0]*m[1][2]*m[2][3])
                )
            }, {
                det * (
                      (m[1][1]*m[2][3]*m[3][0])
                    - (m[1][3]*m[2][1]*m[3][0])
                    + (m[1][3]*m[2][0]*m[3][1])
                    - (m[1][0]*m[2][3]*m[3][1])
                    - (m[1][1]*m[2][0]*m[3][3])
                    + (m[1][0]*m[2][1]*m[3][3])
                ),
                det * (
                      (m[0][3]*m[2][1]*m[3][0])
                    - (m[0][1]*m[2][3]*m[3][0])
                    - (m[0][3]*m[2][0]*m[3][1])
                    + (m[0][0]*m[2][3]*m[3][1])
                    + (m[0][1]*m[2][0]*m[3][3])
                    - (m[0][0]*m[2][1]*m[3][3])
                ),
                det * (
                      (m[0][1]*m[1][3]*m[3][0])
                    - (m[0][3]*m[1][1]*m[3][0])
                    + (m[0][3]*m[1][0]*m[3][1])
                    - (m[0][0]*m[1][3]*m[3][1])
                    - (m[0][1]*m[1][0]*m[3][3])
                    + (m[0][0]*m[1][1]*m[3][3])
                ),
                det * (
                      (m[0][3]*m[1][1]*m[2][0])
                    - (m[0][1]*m[1][3]*m[2][0])
                    - (m[0][3]*m[1][0]*m[2][1])
                    + (m[0][0]*m[1][3]*m[2][1])
                    + (m[0][1]*m[1][0]*m[2][3])
                    - (m[0][0]*m[1][1]*m[2][3])
                )
            }, {
                det * (
                      (m[1][2]*m[2][1]*m[3][0])
                    - (m[1][1]*m[2][2]*m[3][0])
                    - (m[1][2]*m[2][0]*m[3][1])
                    + (m[1][0]*m[2][2]*m[3][1])
                    + (m[1][1]*m[2][0]*m[3][2])
                    - (m[1][0]*m[2][1]*m[3][2])
                ),
                det * (
                      (m[0][1]*m[2][2]*m[3][0])
                    - (m[0][2]*m[2][1]*m[3][0])
                    + (m[0][2]*m[2][0]*m[3][1])
                    - (m[0][0]*m[2][2]*m[3][1])
                    - (m[0][1]*m[2][0]*m[3][2])
                    + (m[0][0]*m[2][1]*m[3][2])
                ),
                det * (
                      (m[0][2]*m[1][1]*m[3][0])
                    - (m[0][1]*m[1][2]*m[3][0])
                    - (m[0][2]*m[1][0]*m[3][1])
                    + (m[0][0]*m[1][2]*m[3][1])
                    + (m[0][1]*m[1][0]*m[3][2])
                    - (m[0][0]*m[1][1]*m[3][2])
                ),
                det * (
                      (m[0][1]*m[1][2]*m[2][0])
                    - (m[0][2]*m[1][1]*m[2][0])
                    + (m[0][2]*m[1][0]*m[2][1])
                    - (m[0][0]*m[1][2]*m[2][1])
                    - (m[0][1]*m[1][0]*m[2][2])
                    + (m[0][0]*m[1][1]*m[2][2])
                )
            }
		};
	}

    float determinant_simple() const noexcept
	{
		// 12 multiplies, 6 adds
		return
			- (m[0][2]*m[1][1]*m[2][0])
			+ (m[0][1]*m[1][2]*m[2][0])
			+ (m[0][2]*m[1][0]*m[2][1])
			- (m[0][0]*m[1][2]*m[2][1])
			- (m[0][1]*m[1][0]*m[2][2])
			+ (m[0][0]*m[1][1]*m[2][2]);
	}

	// Compute inverse transform, assuming it is a
	// 3x3 matrix + translation (4th row = 0 0 0 1)
	mat4x4 inverse_simple() const noexcept
	{
        float det = determinant_simple();
        assert(det != 0);
        if (det == 0)
			return *this;
        det = 1.0f / det;

		// 66 multiplies, 33 adds (before CSE)

        return {
            {
                det * (
                    - (m[1][2]*m[2][1])
                    + (m[1][1]*m[2][2])
                ),
                det * (
                    + (m[0][2]*m[2][1])
                    - (m[0][1]*m[2][2])
                ),
                det * (
                    - (m[0][2]*m[1][1])
                    + (m[0][1]*m[1][2])
                ),
                det * (
                      (m[0][3]*m[1][2]*m[2][1])
                    - (m[0][2]*m[1][3]*m[2][1])
                    - (m[0][3]*m[1][1]*m[2][2])
                    + (m[0][1]*m[1][3]*m[2][2])
                    + (m[0][2]*m[1][1]*m[2][3])
                    - (m[0][1]*m[1][2]*m[2][3])
                )
            }, {
                det * (
                    + (m[1][2]*m[2][0])
                    - (m[1][0]*m[2][2])
                ),
                det * (
                    - (m[0][2]*m[2][0])
                    + (m[0][0]*m[2][2])
                ),
                det * (
                    + (m[0][2]*m[1][0])
                    - (m[0][0]*m[1][2])
                ),
                det * (
                      (m[0][2]*m[1][3]*m[2][0])
                    - (m[0][3]*m[1][2]*m[2][0])
                    + (m[0][3]*m[1][0]*m[2][2])
                    - (m[0][0]*m[1][3]*m[2][2])
                    - (m[0][2]*m[1][0]*m[2][3])
                    + (m[0][0]*m[1][2]*m[2][3])
                )
            }, {
                det * (
                    - (m[1][1]*m[2][0])
                    + (m[1][0]*m[2][1])
                ),
                det * (
                    + (m[0][1]*m[2][0])
                    - (m[0][0]*m[2][1])
                ),
                det * (
                    - (m[0][1]*m[1][0])
                    + (m[0][0]*m[1][1])
                ),
                det * (
                      (m[0][3]*m[1][1]*m[2][0])
                    - (m[0][1]*m[1][3]*m[2][0])
                    - (m[0][3]*m[1][0]*m[2][1])
                    + (m[0][0]*m[1][3]*m[2][1])
                    + (m[0][1]*m[1][0]*m[2][3])
                    - (m[0][0]*m[1][1]*m[2][3])
                )
            }, {
                0, 0, 0, 1
            }    
        };
	}
    
    static mat4x4 rotate_axis(vec3 const& axis, float angleRads) noexcept
	{
        float c, s, t;
		vec3 sv, tv;

        sincosf(angleRads, &s, &c);
		t = 1.0f - c;

		sv = axis * s;
		tv = axis * t;

        return {
            {
                tv.x * axis.x + c,      // txx + c
                tv.x * axis.y - sv.z,	// txy - sz
                tv.x * axis.z + sv.y,	// txz + sy
                0.0f
            }, {
                tv.x * axis.y + sv.z,	// txy + sz
                tv.y * axis.y + c,		// tyy + c
                tv.y * axis.z - sv.x,	// tyz - sx
                0.0f
            }, {
                tv.x * axis.z - sv.y,	// txz - sy
                tv.y * axis.z + sv.x,	// tyz + sx
                tv.z * axis.z + c,		// tzz + c
                0.0f
            }, {
                0.0f, 0.0f, 0.0f, 1.0f
            }
        };
	}
	
	static mat4x4 scale4(float x, float y, float z, float w)
	{
        return {
            {    x, 0.0f, 0.0f, 0.0f },
            { 0.0f,    y, 0.0f, 0.0f },
            { 0.0f, 0.0f,    z, 0.0f },
            { 0.0f, 0.0f, 0.0f,    w }
        };
	}
	
	static mat4x4 scale(float s)
	{
        return {
            {    s, 0.0f, 0.0f, 0.0f },
            { 0.0f,    s, 0.0f, 0.0f },
            { 0.0f, 0.0f,    s, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
	}
	
	static mat4x4 rotate_x(float a)
	{
        float s, c;
        sincosf(a, &s, &c);
        return {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f,    c,   -s, 0.0f },
            { 0.0f,    s,    c, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
	}
	
	static mat4x4 rotate_y(float a)
	{
        float s, c;
        sincosf(a, &s, &c);
        return {
            {    c, 0.0f,    s, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            {   -s, 0.0f,    c, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
	}
	
	static mat4x4 rotate_z(float a)
	{
        float s, c;
        sincosf(a, &s, &c);
        return {
            {    c,   -s, 0.0f, 0.0f },
            {    s,    c, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
	}

	static constexpr mat4x4 translate(vec3 const& v)
	{
        return {
            { 1.0f, 0.0f, 0.0f,  v.x },
            { 0.0f, 1.0f, 0.0f,  v.y },
            { 0.0f, 0.0f, 1.0f,  v.z },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
	}

    static constexpr mat4x4 perspective(float l, float t,
                                        float r, float b,
                                        float n, float f)
    {
        float n2 = 2 * n;
        float rpl = r + l;
        float rml = r - l;
        float tpb = t + b;
        float tmb = t - b;
        float fpn = f + n;
        float fmn = f - n;
        float x = n2 / rml;
        float y = n2 / tmb;
        float A = rpl / rml;
        float B = tpb / tmb;
        float C = -fpn / fmn;
        float D = -(f * n2) / fmn;
        return {
            {    x, 0.0f, 0.0f,  0.0f },
            { 0.0f,    y, 0.0f,  0.0f },
            {    A,    B,    C, -1.0f },
            { 0.0f, 0.0f,    D,  0.0f }
        };
    }
    
    void transform(vec3 * __restrict v, size_t count)
    {
        for (size_t i = 0; i < count; ++i) {
            v[i].w = 1.0f;
            v[i] = {
                v[i].x*m[0][0]+v[i].y*m[0][1]+v[i].z*m[0][2]+v[i].w*m[0][3],
                v[i].x*m[1][0]+v[i].y*m[1][1]+v[i].z*m[1][2]+v[i].w*m[1][3],
                v[i].x*m[2][0]+v[i].y*m[2][1]+v[i].z*m[2][2]+v[i].w*m[2][3],
                v[i].x*m[3][0]+v[i].y*m[3][1]+v[i].z*m[3][2]+v[i].w*m[3][3]
            };
        }
    }
};

