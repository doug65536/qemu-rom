#pragma once

#include <stdint.h>
#include <stddef.h>

bool bochs_dispi_init(uintptr_t mmio_addr, 
        uintptr_t framebuffer_addr, size_t framebuffer_size);

bool bochs_dispi_set_pos(size_t index, int x, int y);

bool bochs_dispi_set_enable(size_t index, bool enable, bool noclear);

bool bochs_dispi_set_mode(size_t index,
        int w, int h, int bpp, 
        int x = 0, int y = 0, 
        int vw = -1, int vh = -1,
        bool enabled = true,
        bool noclear = false);
