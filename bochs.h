#pragma once

#include <stdint.h>
#include <stddef.h>

bool bochs_dispi_init(uintptr_t mmio_addr, 
        uintptr_t framebuffer_addr, size_t framebuffer_size);
