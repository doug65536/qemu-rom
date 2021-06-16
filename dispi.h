#pragma once

#include <stdint.h>
#include <stddef.h>

bool dispi_init();

// If ioport_base is nonzero, it is the address of two 16 bit fields,
// a 16-bit select register and 16-bit data register
bool dispi_add_device(uintptr_t mmio_addr, 
        uintptr_t framebuffer_addr, size_t framebuffer_size);

void dispi_add_mmio_device(
    uintptr_t framebuffer_addr, size_t framebuffer_size, 
    uintptr_t mmio_addr);

size_t dispi_display_count();

bool dispi_set_mode(size_t index,
        int w, int h, int bpp, 
        int x = 0, int y = 0, 
        int vw = -1, int vh = -1,
        bool enabled = true,
        bool noclear = false);

bool dispi_set_pos(size_t index, int x, int y);

bool dispi_set_enable(size_t index, bool enable, bool noclear);

bool dispi_fill_screen(size_t index, size_t page);

struct dispi_framebuffer_t {
    uint32_t *pixels;
    size_t pitch;
    size_t width;
    size_t height;
};

bool dispi_get_framebuffer(size_t index, dispi_framebuffer_t *info);
