#include <stddef.h>
#include <stdint.h>
#include "pci.h"
#include "assert.h"
#include "debug.h"
#include "dispi.h"

void pci_init()
{
    int bus_todo[256];
    int *todo_end = bus_todo + 256;
    int *todo_ptr = todo_end;

    *--todo_ptr = 0;

    uint32_t mm_base = 0xf0000000;
    uint32_t io_base = 0xf000;

    while (todo_ptr < todo_end) {
        int bus = *todo_ptr++;

        for (int slot = 0; slot < 32; ++slot) {
            int func_count = 1;

            for (int func = 0; func < func_count; ++func) {
                pci_addr addr{bus, slot, func};
                
                printdbg("Checking %d:%d:%d\n", bus, slot, func);

                uint8_t header_type = pci_read(addr, sizeof(uint8_t),
                            offsetof(pci_config_hdr_t, header_type));
                
                if (header_type == 0xFF)
                    break;

                if (func == 0) {
                    bool multifunction = header_type & 0x80;

                    // Extend function loop if header type says multifunction
                    if (multifunction)
                        func_count = 8;
                }

                uint8_t dev_class = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, dev_class));

                if (dev_class == 0xFF)
                    break;

                uint8_t subclass = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, subclass));

                uint8_t pif = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, prog_if));
                
                
                char const *description = pci_describe_device(
                        dev_class, subclass, pif);
                
                printdbg("class=%x subclass=%x %s\n", dev_class, subclass,
                        description);

                if (dev_class == PCI_DEV_CLASS_BRIDGE &&
                        subclass == PCI_SUBCLASS_BRIDGE_PCI2PCI) {
                    uint8_t secondary_bus = pci_read(addr, sizeof(uint8_t),
                            offsetof(pci_config_hdr_t, base_addr[2]) + 1);

                    if (todo_ptr > bus_todo)
                        *--todo_ptr = secondary_bus;
                } else if (dev_class == PCI_DEV_CLASS_DISPLAY) {
                    assert(!(header_type & 0x7F));
                    
                    uint32_t bar_alignments[5];

                    set_bars(addr, mm_base, io_base, bar_alignments);
                    
                    uint32_t dispi_mmio_bar = pci_read(addr, sizeof(uint32_t),
                            offsetof(pci_config_hdr_t, base_addr) +
                            sizeof(uint32_t) * 2);

                    uint32_t dispi_fb_bar = pci_read(addr, sizeof(uint32_t),
                            offsetof(pci_config_hdr_t, base_addr) +
                            sizeof(uint32_t) * 0);

                    uint32_t dispi_mmio_base = dispi_mmio_bar & -16;
                    uint32_t dispi_fb_base = dispi_fb_bar & -16;

                    dispi_init(dispi_mmio_base, 
                            dispi_fb_base, bar_alignments[0]);

                    continue;
                }

                if (!(header_type & 0x7F))
                    set_bars(addr, mm_base, io_base, nullptr);
            }
        }
    }
}

int main()
{
    pci_init();
    
    size_t display_count = dispi_display_count();
    
    for (size_t i = 0; i < display_count; ++i) {
        int width = 1024;
        int height = 768;

        dispi_set_mode(i, width, height, 32);

        dispi_fill_screen(i);
    }
    
    return 0;
}
