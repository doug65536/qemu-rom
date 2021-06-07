#include "arch/pci.h"

arch_mmio_range_t arch_mmio_range()
{
    return { 0, 0 };
}

uint32_t arch_pci_read(pci_addr const &/*addr*/, uint32_t /*offset*/)
{
    return (uint32_t)-1;
}

void arch_pci_write(pci_addr const &/*addr*/, 
        uint32_t /*offset*/, uint32_t /*value*/)
{
}
