#include <stddef.h>
#include <stdint.h>
#include "arch/pci.h"
#include "assert.h"
#include "debug.h"
#include "dispi.h"
#include "polygon.h"
#include "likely.h"
#include "math/math.h"
#include "vec.h"
#include "malloc.h"

vec4 test_cube[] = {
    // South face
    { -1.0f,  1.0f,  1.0f },    // lt
    { -1.0f, -1.0f,  1.0f },    // bl
    {  1.0f, -1.0f,  1.0f },    // br
    {  1.0f,  1.0f,  1.0f },    // tr
    
    // North face
    {  1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f }
};

uint32_t test_indexes[] = {
    0, 1, 2, 3,
    4, 5, 6, 7,
    0, 5, 6, 1,
    3, 4, 5, 2
};

// T-C-N-V
//

// 2D
//GL_V2F
//GL_C4UB_V2F
//
//// 3D
//GL_V3F
//GL_C4UB_V3F
//GL_N3F_V3F
//
//// 3d with 2d texcoords
//GL_T2F_V3F
//GL_T2F_C4UB_V3F
//GL_T2F_N3F_V3F
//
//// homogeneous
//GL_T4F_V4F
//
//// excessive colour resolution
//GL_C3F_V3F
//GL_C4F_N3F_V3F
//GL_T2F_C3F_V3F
//GL_T2F_C4F_N3F_V3F
//GL_T4F_C4F_N3F_V4F

struct vertex {
    texcoord tex;
    vec4 pos;
};

extern void *bump_alloc;

int main();
int main()
{
    //*(int*)0xf00ff00f = 42;
    pci_init();
    
    void *heap_st = bump_alloc;
    void *heap_en = (char*)heap_st + (4 << 20);
    bump_alloc = heap_en;
    malloc_init(heap_st, heap_en);
    
    bool dispi_ok = dispi_init();
    
    if (!dispi_ok)
        return 0;
    
    size_t display_count = dispi_display_count();
    
    for (size_t i = 0; i < display_count; ++i) {
        int width = 1024;
        int height = 768;

        dispi_set_mode(i, width, height, 32);

        dispi_fill_screen(i, 0);
    }
    
    if (display_count) {
        dispi_framebuffer_t fb;
        if (dispi_get_framebuffer(0, &fb)) {
            matstk mtxView, mtxProj;
            mtxView.look_at(vec4(0, 0, 10), vec4(), vec4(0, 1, 0));
        
            //mtxProj.load(mat4x4::perspective(-1.0f, 1.0f, 
            //      1.0f, -1.0f, 1.0f, 1024.0f));
            mtxProj.load(mat4x4::perspective(
                    -1, 1, 1, -1, 1, 1024));
            
            vec4 test[] = {
                {  0.0f,  0.0f,    -6.0f },
                { -1.0f, -1.0f,     6.0f },
                {  1.0f, -1.0f,     6.0f }
            };
            static constexpr size_t test_vec_count = 
                    sizeof(test)/sizeof(*test);
            vec4 xf[test_vec_count];
            
            set_render_surface(fb.pixels, fb.pitch, fb.width, fb.height);
            
//            float pix100 = 314.15926535897923f;
            for (size_t i = 0; i < 0xffffff; ++i) {
                mtxProj->transform(xf, test, test_vec_count);
                
                // Clip
                int clip_m[4] = { 0, 0, 0, 0 };
                int clip_p[4] = { 0, 0, 0, 0 };
                
                //
                // Classify all the points
                
                for (size_t i = 0; i < test_vec_count; ++i) {
                    vec4 &v = xf[i];
                    float pw = v.w;
                    float mw = -v.w;
                    
                    // Say it in a way that makes it obvious to vectorize
                    clip_p[0] += v.x > pw;
                    clip_p[1] += v.y > pw;
                    clip_p[2] += v.z > pw;
                    clip_p[3] += v.w > pw;
                    
                    clip_m[0] += v.x < mw;
                    clip_m[1] += v.y < mw;
                    clip_m[2] += v.z < mw;
                    clip_m[3] += v.w < mw;
                }
                
                //
                // Skip if completely clipped away
                
                if (clip_m[0] == test_vec_count || 
                        clip_m[1] == test_vec_count ||
                        clip_m[2] == test_vec_count ||
                        clip_m[3] == test_vec_count)
                    continue;
                
                if (clip_p[0] == test_vec_count || 
                        clip_p[1] == test_vec_count ||
                        clip_p[2] == test_vec_count ||
                        clip_p[3] == test_vec_count)
                    continue;
                
                vec4 outverts[4];
                size_t outindex = 0;
                
                if (clip_m[2]) {
                    // +z clip
                    
                    for (size_t i = 1; i <= test_vec_count; ++i) {
                        vec4 const& st = xf[i - 1];
                        vec4 const& en = xf[i & -(i < test_vec_count)];
                        vec4 de = en - st;
                        float d;
                        
                        //   3 ----+--+--
                        //         | /  |  
                        //         |/   | down -2 -  3 = -5
                        //   0 ----*    |  
                        //        /     |   up  3 - -2 =  5
                        //  -2 --+-------
                        
                        if ((st.w+st.z) >= 0 && (en.w+en.z) < 0) {
                            // Clip end
                            d = (st.w+st.z) / ((st.w+st.z) - (en.w+en.z));
                            outverts[outindex++] = st;
                            outverts[outindex++] = st + de * d;
                        } else if ((st.w+st.z) < 0 && (en.w+en.z) >= 0) {
                            // Clip start
                            d = -(st.w+st.z) / ((en.w+en.z) - (st.w+st.z));
                            outverts[outindex++] = st + de * d;
                        } else if ((st.w+st.z) > 0) {
                            outverts[outindex++] = st;
                        }
                    }
                }
                
                for (size_t i = 0; i < outindex; ++i)
                    xf[i] = outverts[i];
                size_t vec_count = outindex;
                
                // Transform normalized device coordinates 
                // into window coordinates
                for (size_t i = 0; i < vec_count; ++i) {
                    vec4 v = xf[i];
                    
                    v.w = 1.0f / v.w;
                    
                    v = {
                        (v.x * v.w + 1.0f) * fb.width * 0.5f,
                        (v.y * v.w + 1.0f) * fb.height * 0.5f,
                        (v.z + 1.0f) * 0.5f * v.w,
                        v.w
                    };
                    
                    xf[i] = v;
                }
                
                draw_tri_ccw(xf, xf+2, xf+1, i);
            }
        }
    }
    
    return 0;
}
