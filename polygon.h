#pragma once
#include "vec.h"

void set_render_surface(uint32_t *pixels, uint32_t pitch, 
    uint32_t width, uint32_t height);

void draw_tri_ccw(vec3 const *v0, vec3 const *v1, vec3 const *v2, 
    uint32_t color);
