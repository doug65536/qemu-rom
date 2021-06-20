#pragma once

#include "math/math.h"

struct texcoord {
    float u, v;

    constexpr texcoord() : u(0), v(0) {}
    constexpr texcoord(float u, float v) : u(u), v(v) {}

    texcoord operator*(float rhs)
    {
        return {
            u * rhs,
            v * rhs
        };
    }

    texcoord operator-(texcoord const &rhs)
    {
        return {
            u - rhs.u,
            v - rhs.v
        };
    }

    texcoord operator+(texcoord const &rhs)
    {
        return {
            u + rhs.u,
            v + rhs.v
        };
    }
};

struct vec4 {
    float x, y, z, w;

    constexpr vec4() : x(0), y(0), z(0), w(0) {}

    constexpr vec4(vec4 const& v) = default;

    constexpr vec4(float x, float y, float z, float w = 1.0f)
        : x(x), y(y), z(z), w(w) {}

    constexpr explicit vec4(float n)
        : x(n), y(n), z(n), w(n) {}

    constexpr vec4 &operator=(vec4 const& rhs) = default;

    vec4 operator+(vec4 const& rhs) const
    {
        return {
            x + rhs.x,
            y + rhs.y,
            z + rhs.z,
            w + rhs.w
        };
    }

    vec4 &operator+=(vec4 const& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    vec4 operator-(vec4 const& rhs) const
    {
        return {
            x - rhs.x,
            y - rhs.y,
            z - rhs.z,
            w - rhs.w
        };
    }

    vec4 &operator-=(vec4 const& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    vec4 operator*(vec4 const& rhs) const
    {
        return {
            x * rhs.x,
            y * rhs.y,
            z * rhs.z,
            w * rhs.w
        };
    }

    vec4 &operator*=(vec4 const& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    vec4 operator*(float rhs) const
    {
        return {
            x * rhs,
            y * rhs,
            z * rhs,
            w * rhs
        };
    }

    vec4 &operator*=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }

    vec4 operator/(float rhs) const
    {
        rhs = 1.0f / rhs;
        return {
            x * rhs,
            y * rhs,
            z * rhs,
            w * rhs
        };
    }

    vec4 &operator/=(float rhs)
    {
        rhs = 1.0f / rhs;
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }

    vec4 cross(vec4 const& rhs) const
    {
        return {
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x,
            0
        };
    }

    float dot(vec4 const& rhs) const
    {
        return x * rhs.x + y * rhs.y + z;
    }

    float sq_len() const
    {
        return x * x + y * y + z * z;
    }

    float len() const
    {
        return sqrtf(sq_len());
    }

    float recip_len() const
    {
        return rsqrtf(sq_len());
    }

    vec4 &normalize()
    {
        float rl = 1.0f / len();
        x *= rl;
        y *= rl;
        z *= rl;
        w *= rl;
        return *this;
    }

    vec4 &normalize_fast()
    {
        float rl = recip_len();
        x *= rl;
        y *= rl;
        z *= rl;
        w *= rl;
        return *this;
    }

    vec4 normalized_fast() const
    {
        float rl = recip_len();
        return {
            x * rl,
            y * rl,
            z * rl,
            w * rl
        };
    }
    
    // 0=x, 1=-x, 2=y, 3=-y, 4=z, 5=-z
    float dot_clip_plane(int plane) const
    {
        switch (plane) {
        case 0: return  x + w;
        case 1: return -x + w;
        case 2: return  y + w;
        case 3: return -y + w;
        case 4: return  z + w;
        case 5: return -z + w;
        }
        return 0;
    }
    
    int outcode() const
    {
        int result = 0;
        
        for (size_t plane = 0; plane < 6; ++plane)
            result |= (dot_clip_plane(plane) < 0.0f) << plane;
        
        return result;
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

    constexpr mat4x4(vec4 const &a, vec4 const &b,
            vec4 const &c, vec4 const &d)
        : m{
            { a.x, a.y, a.z, a.w },
            { b.x, b.y, b.z, b.w },
            { c.x, c.y, c.z, c.w },
            { d.x, d.y, d.z, d.w }
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

    static mat4x4 rotate_axis(vec4 const& axis, float angleRads) noexcept
    {
        float c, s, t;
        vec4 sv, tv;

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

    static constexpr mat4x4 translate(vec4 const& v)
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
        float rml = r - l;
        float rpl = r + l;
        float tmb = t - b;
        float tpb = t + b;
        float fmn = f - n;
        float fpn = f + n;
        float x = n2 / rml;
        float y = n2 / tmb;
        float A = rpl / rml;
        float B = tpb / tmb;
        float C = -fpn / fmn;
        float D = -(f * n2) / fmn;
        return {
//            {    x, 0.0f, 0.0f,  0.0f },
//            { 0.0f,    y, 0.0f,  0.0f },
//            {    A,    B,    C, -1.0f },
//            { 0.0f, 0.0f,    D,  0.0f }
            {    x,  0,  A, 0 },
            { 0.0f,  y,  B, 0 },
            {    0,  0,  C, D },
            { 0.0f,  0, -1, 0 }
        };
    }

    void transform(vec4 *dst, vec4 *src, size_t count) const
    {
        for (size_t i = 0; i < count; ++i) {
            vec4 p = src[i];
            p.w = 1.0f;
            p = {
                p.x * m[0][0] + p.y * m[0][1] + p.z * m[0][2] + p.w * m[0][3],
                p.x * m[1][0] + p.y * m[1][1] + p.z * m[1][2] + p.w * m[1][3],
                p.x * m[2][0] + p.y * m[2][1] + p.z * m[2][2] + p.w * m[2][3],
                p.x * m[3][0] + p.y * m[3][1] + p.z * m[3][2] + p.w * m[3][3]
            };
            dst[i] = p;
        }
    }
};

class matstk {
    size_t sp = 0;
    mat4x4 stack[64];

public:
    operator mat4x4() const
    {
        return stack[sp];
    }
    
    mat4x4 const *operator->() const
    {
        return stack + sp;
    }
    
    mat4x4 *operator->()
    {
        return stack + sp;
    }
    
    mat4x4 const &operator *() const
    {
        return stack[sp];
    }
    
    mat4x4 &operator *()
    {
        return stack[sp];
    }
    
    matstk &load(mat4x4 const& rhs)
    {
        stack[sp] = rhs;
        return *this;
    }

    matstk &reset()
    {
        sp = 0;
        return identity();
    }

    matstk &push()
    {
        assert(sp + 1 < sizeof(stack) / sizeof(*stack));
        ++sp;
        return load(stack[sp - 1]);
    }

    matstk &pop()
    {
        assert(sp > 0);
        --sp;
        return *this;
    }

    matstk &mul(mat4x4 const& transformation)
    {
        return load(transformation * stack[sp]);
    }

    matstk &rotate_axis(vec4 const& axis, float angleRads)
    {
        return mul(mat4x4::rotate_axis(axis, angleRads));
    }

    matstk &identity()
    {
        return load(mat4x4());
    }

    matstk &rotate_x(float angleRads)
    {
        return mul(mat4x4::rotate_x(angleRads));
    }

    matstk &rotate_y(float angleRads)
    {
        return mul(mat4x4::rotate_y(angleRads));
    }

    matstk &rotate_z(float angleRads)
    {
        return mul(mat4x4::rotate_z(angleRads));
    }

    matstk &scale(float s)
    {
        return mul(mat4x4::scale(s));
    }

    matstk &look_at(vec4 const& pos,
                 vec4 const& lookAt,
                 vec4 const& up)
    {
        vec4 p = pos;
        vec4 row2 = pos - lookAt;  // the view Z vector points back
        vec4 row1;
        vec4 row0;

        if (row2.sq_len() < 0.1f)
        {
            // Nudge the camera
            p.z += 0.2f;
            row2 = p - lookAt;
        }

        row2.normalize();

        // Calculate the right vector from the up and back vector
        row0 = up.cross(row2);
        row0.normalize();

        // Calculate the up vector from the back and right vector
        row1 = row2.cross(row0);
        row1.normalize();

        return from_dirs_and_pos(row0, row1, row2, p);
    }

    matstk &from_dirs_and_pos(
        vec4 const& vx, vec4 const& vy, vec4 const& vz, 
        vec4 const& vp)
    {
        return load({
            { vx.x, vx.y, vx.z, -vp.dot(vx) },
            { vy.x, vy.y, vy.z, -vp.dot(vy) },
            { vz.x, vz.y, vz.z, -vp.dot(vz) },
            { 0.0f, 0.0f, 0.0f,        1.0f }
        });
    }
};
