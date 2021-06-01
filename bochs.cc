#include <stddef.h>
#include "bochs.h"
#include "debug.h"

// https://gitlab.com/qemu-project/qemu/-/blob/master/docs/specs/standard-vga.txt#L59

// MMIO
// Likewise applies to the pci variant only for obvious reasons.
// 
// 0000 - 03ff : edid data blob.
// 0400 - 041f : vga ioports (0x3c0 -> 0x3df), remapped 1:1.
//               word access is supported, bytes are written
//               in little endia order (aka index port first),
//               so indexed registers can be updated with a
//               single mmio write (and thus only one vmexit).
// 0500 - 0515 : bochs dispi interface registers, mapped flat
//               without index/data ports.  Use (index << 1)
//               as offset for (16bit) register access.
// 
// 0600 - 0607 : qemu extended registers.  qemu 2.2+ only.
//               The pci revision is 2 (or greater) when
//               these registers are present.  The registers
//               are 32bit.
//   0600      : qemu extended register region size, in bytes.
//   0604      : framebuffer endianness register.
//               - 0xbebebebe indicates big endian.
//               - 0x1e1e1e1e indicates little endian.

#define VBE_DISPI_INDEX_ID              0x0
#define VBE_DISPI_INDEX_XRES            0x1
#define VBE_DISPI_INDEX_YRES            0x2
#define VBE_DISPI_INDEX_BPP             0x3
#define VBE_DISPI_INDEX_ENABLE          0x4
#define VBE_DISPI_INDEX_BANK            0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH      0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT     0x7
#define VBE_DISPI_INDEX_X_OFFSET        0x8
#define VBE_DISPI_INDEX_Y_OFFSET        0x9

struct dispi_mmio_t {
    uint16_t id;
    uint16_t xres;
    uint16_t yres;
    uint16_t bpp;
    uint16_t enable;
    uint16_t bank;
    uint16_t virt_width;
    uint16_t virt_height;
    uint16_t x_offset;
    uint16_t y_offset;
};

//static dispi_mmio_t volatile *dispi_mmio;

#define VBE_DISPI_BPP_4                  0x04
#define VBE_DISPI_BPP_8                  0x08
#define VBE_DISPI_BPP_15                 0x0F
#define VBE_DISPI_BPP_16                 0x10
#define VBE_DISPI_BPP_24                 0x18
#define VBE_DISPI_BPP_32                 0x20

#define VBE_DISPI_DISABLED               0x00
#define VBE_DISPI_ENABLED                0x01
#define VBE_DISPI_GETCAPS                0x02
#define VBE_DISPI_8BIT_DAC               0x20
#define VBE_DISPI_LFB_ENABLED            0x40
#define VBE_DISPI_NOCLEARMEM             0x80

struct display_t {
    uintptr_t framebuffer_addr;
    uintptr_t framebuffer_size;
    uintptr_t mmio_addr;
};

// QEMU segfaults with more than 7 anyway
static constexpr size_t MAX_DISPLAYS = 8;
static display_t displays[MAX_DISPLAYS];
static size_t display_count;

bool bochs_dispi_init(uintptr_t mmio_addr, 
        uintptr_t framebuffer_addr, size_t framebuffer_size)
{
    if (display_count >= MAX_DISPLAYS)
        return false;
    
    dispi_mmio_t volatile *dispi_mmio;
    
    uint8_t volatile *mmio = (uint8_t volatile *)mmio_addr;
    
    // Unblank
    mmio[0x400] = 0x20;
    
    printdbg("Using dispi MMIO at %x\n", mmio_addr);
    
    dispi_mmio = (dispi_mmio_t*)(mmio_addr + 0x500);
    
    //dispi_mmio->id = 0xB0C5;
    dispi_mmio->xres = 1024;
    dispi_mmio->yres = 768;
    dispi_mmio->bpp = 32;
    dispi_mmio->bank = 0;
    dispi_mmio->virt_width = 1024 * (dispi_mmio->bpp/8);
    dispi_mmio->virt_height = 768;
    dispi_mmio->x_offset = 0;
    dispi_mmio->y_offset = 0;
    dispi_mmio->enable = VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED;
    
    displays[display_count++] = {
        framebuffer_addr,
        framebuffer_size,
        mmio_addr
    };
    
    printdbg("Initialized %uKB display at %zx\n", 
            framebuffer_size >> 10, framebuffer_addr);
    
    return true;
}
