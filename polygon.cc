#include "polygon.h"
#include <stdint.h>

struct render_surface_t {
    uint32_t *pixels;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
};

render_surface_t render_surface;

static constexpr size_t MAX_VRES = 2160;
static uint16_t scratch16[MAX_VRES];

void set_render_surface(uint32_t *pixels, uint32_t pitch, 
    uint32_t width, uint32_t height)
{
    render_surface.pixels = pixels;
    render_surface.pitch = pitch;
    render_surface.width = width;
    render_surface.height = height;
}

static void draw_tri_scan_edge(
    uint16_t *left_output, uint16_t *right_output,
    vec3 const *v0, vec3 const *v1, int miny)
{
    // Counterclockwise triangles go down on the left, and up on the right
    uint16_t *output = v0->y < v1->y ? left_output : right_output;
    vec3 const *vs;
    vec3 const *ve;

    if (v0->y < v1->y) {
        // Left side
        output = left_output;
        vs = v0;
        ve = v1;
    } else {
        // Right side
        output = right_output;
        vs = v1;
        ve = v0;
    }
    
    int sy = (int)vs->y;
    int ey = (int)ve->y;
    int offset = (int)vs->y - miny;
    output += offset;
    
    int fpsx = (int)(vs->x * 65536.0f);
    int fpex = (int)(ve->x * 65536.0f);
    int fpdx = fpex - fpsx;
    int dy = ve->y - vs->y;
    int step = fpdx / dy;
    // Add rounding offset outside loop, so loop can just truncate
    int x = fpsx + 0x8000;
    for (int i = sy; i < ey; ++i, x += step)
        *output++ = x >> 16;
}

static void fill_tri(
    uint16_t const *left_output, uint16_t const *right_output, 
    int miny, int maxy, uint32_t color)
{
    uint32_t *scanline = (uint32_t*)((char*)render_surface.pixels + 
        (render_surface.pitch * miny));
    
    while (miny++ < maxy) {
        size_t en = *right_output++;
        size_t st = *left_output++;
        while (st < en)
            scanline[st++] = color;
        scanline = (uint32_t*)((char*)scanline + render_surface.pitch);
    }
}

void draw_tri_ccw(vec3 const *v0, vec3 const *v1, vec3 const *v2, 
    uint32_t color)
{
    // There are possibilities:
    //  1) v0 is already at the top, and is already first
    //  2) v1 should be first
    //  3) v2 should be first
    
    float minyf, maxyf;
    if (v0->y < v1->y) {
        // v0 is lower. use v0 to get min, use v1 to get max
        minyf = v0->y < v2->y ? v0->y : v2->y;
        maxyf = v1->y > v2->y ? v1->y : v2->y;
    } else {
        // v1 lower or equal. use v1 to get min, use v0 to get max
        minyf = v1->y < v2->y ? v1->y : v2->y;
        maxyf = v0->y > v2->y ? v0->y : v2->y;
    }

    int miny = (int)minyf;
    int maxy = (int)maxyf;
    
    int height = maxy - miny;
    
    uint16_t *left_output = scratch16;
    uint16_t *right_output = scratch16 + height;
    
    if (v0->y != v1->y)
        draw_tri_scan_edge(left_output, right_output, v0, v1, miny);
    if (v1->y != v2->y)
        draw_tri_scan_edge(left_output, right_output, v1, v2, miny);
    if (v2->y != v0->y)
        draw_tri_scan_edge(left_output, right_output, v2, v0, miny);
    
    fill_tri(left_output, right_output, miny, maxy, color);
}
