#include <stddef.h>
#include <stdint.h>
#include "arch/pci.h"
#include "assert.h"
#include "debug.h"
#include "dispi.h"
#include "polygon.h"

int main();
int main()
{
    //*(int*)0xf00ff00f = 42;
    pci_init();
    
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
            set_render_surface(fb.pixels, fb.pitch, fb.width, fb.height);
            
            for (size_t i = 0; i < 0xffffff; ++i) {
                vec3 v0{100, 100, 1};
                vec3 v1{50, 200, 1};
                vec3 v2{150, 150, 1};
                draw_tri_ccw(&v0, &v1, &v2, i);
            }
        }
    }
    
    return 0;
}
