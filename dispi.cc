#include "dispi.h"
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

// 0x18
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

    // From bochs implementation
    uint16_t mem_64k;
    uint16_t index_ddc;
};

struct bochs_vbe_mmio_t {
    uint8_t edid[0x400];

    uint8_t vga_3c0[0x20];
    uint8_t unused[0x500 - 0x420];

    dispi_mmio_t vbe;

    uint8_t unused2[0x600 - (0x500 + sizeof(dispi_mmio_t))];

    // When PCI revision is 2 or greater, these are present
    uint32_t qemu_ext_size_bytes;
    uint32_t framebuffer_endianness;    // 0xbebebebe or 0x1e1e1e1e
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
    unsigned width;
    unsigned height;
    unsigned bpp;
    unsigned virtw;
    unsigned virth;
    bochs_vbe_mmio_t volatile *mmio_addr;
};

// QEMU segfaults with more than 7 anyway
static constexpr size_t MAX_DISPLAYS = 8;
static display_t displays[MAX_DISPLAYS];
static size_t display_count;

bool dispi_fill_screen(size_t index)
{
    if (display_count >= MAX_DISPLAYS)
        return false;

    display_t *display = displays + index;
    uint32_t *pixels = (uint32_t*)display->framebuffer_addr;

    int pixel_count = display->width * display->height;
    for (int i = 0; i < pixel_count; ++i) {
        uint32_t pixel = (!!((i / display->width) & 0x40) ^ !!(i & 0x40)
                ? 0x123456
                : 0x654321);

        if (index & 1)
            pixel ^= 0x44;

        if (index & 2)
            pixel ^= 0x4400;

        if (index & 4)
            pixel ^= 0x440000;

        pixels[i] = pixel;
    }

    return true;
}

bool dispi_init(uintptr_t mmio_addr,
        uintptr_t framebuffer_addr, size_t framebuffer_size)
{
    if (display_count >= MAX_DISPLAYS)
        return false;

    bochs_vbe_mmio_t volatile *mmio = (bochs_vbe_mmio_t volatile *)mmio_addr;

    // Unblank
    mmio->vga_3c0[0] = 0x20;

    printdbg("Using dispi MMIO at %x\n", mmio_addr);

    displays[display_count++] = {
        framebuffer_addr,
        framebuffer_size,
        0, 0, 0, 0, 0,
        mmio
    };

    printdbg("Initialized %uKB display at %zx\n",
            framebuffer_size >> 10, framebuffer_addr);

    return true;
}

size_t dispi_display_count()
{
    return display_count;
}

bool dispi_set_mode(size_t index,
        int w, int h, int bpp,
        int x, int y, int vw, int vh,
        bool enabled, bool noclear)
{
    if (index >= display_count)
        return false;

    if (!vw)
        vw = w * (bpp / 8);

    if (!vh)
        vh = h;

    bochs_vbe_mmio_t volatile *mmio = displays[index].mmio_addr;

    mmio->vbe.enable = 0;
    mmio->vbe.xres = w;
    mmio->vbe.yres = h;
    mmio->vbe.bpp = bpp;
    mmio->vbe.bank = 0;
    mmio->vbe.x_offset = x;
    mmio->vbe.y_offset = y;
    mmio->vbe.virt_width = vw;
    mmio->vbe.virt_height = vh;
    
    displays[index].width = w;
    displays[index].height = h;
    displays[index].bpp = h;
    displays[index].virtw = h;
    displays[index].virth = h;

    return dispi_set_enable(index, enabled, noclear);
}

bool dispi_set_enable(size_t index, bool enabled, bool noclear)
{
    if (index >= display_count)
        return false;

    bochs_vbe_mmio_t volatile *mmio = displays[index].mmio_addr;

    mmio->vbe.enable = enabled
        ? VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED |
            (-noclear & VBE_DISPI_NOCLEARMEM)
        : VBE_DISPI_DISABLED;

    return true;
}

bool dispi_set_pos(size_t index, int x, int y)
{
    if (index >= display_count)
        return false;

    displays[index].mmio_addr->vbe.x_offset = x;
    displays[index].mmio_addr->vbe.y_offset = y;

    return true;
}
