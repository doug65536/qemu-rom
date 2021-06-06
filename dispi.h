#pragma once

#include <stdint.h>
#include <stddef.h>

bool dispi_init();

bool dispi_add_device(uintptr_t mmio_addr, 
        uintptr_t framebuffer_addr, size_t framebuffer_size);

size_t dispi_display_count();

bool dispi_set_mode(size_t index,
        int w, int h, int bpp, 
        int x = 0, int y = 0, 
        int vw = -1, int vh = -1,
        bool enabled = true,
        bool noclear = false);

bool dispi_set_pos(size_t index, int x, int y);

bool dispi_set_enable(size_t index, bool enable, bool noclear);

bool dispi_fill_screen(size_t index);

