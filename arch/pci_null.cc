#include "arch/pci.h"

void pci_init()
{
}

uint32_t arch_pci_read(const pci_addr &/*addr*/, uint32_t /*offset*/)
{
    return (uint32_t)-1;
}

void arch_pci_write(const pci_addr &/*addr*/, uint32_t /*offset*/, 
        uint32_t /*value*/)
{
}
