#include "dispi.h"
#include "arch/pci.h"

bool dispi_init()
{
    size_t index = -1;
    do {
        index = pci_enum_next_vendor_device(index, 0x1234, 0x1111);
        if (index != size_t(-1)) {
            uint64_t framebuffer_bar = pci_bar_get_base(index, 0);
            uint64_t framebuffer_sz = pci_bar_size(index, 0);
            uint64_t mmio_bar = pci_bar_get_base(index, 2);
            
            if (!dispi_add_device(mmio_bar, framebuffer_bar, framebuffer_sz))
                return false;
        }
    } while (index != size_t(-1));
    
    return true;
}
