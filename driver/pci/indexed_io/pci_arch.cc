#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "debug.h"
#include "arch/pci.h"
#include "assert.h"
#include "config.h"

#define PCI_ACCESS_ADDR  (*(uint32_t volatile *)PCICONFIDX_ST)
#define PCI_ACCESS_DATA  (*(uint32_t volatile *)PCICONFDAT_ST)

arch_mmio_range_t arch_mmio_range()
{
    return {
#if defined(HOLE_ST) && defined(HOLE_LB)
        HOLE_ST,
        HOLE_LB+1
#else
        0x80000000,
        0xE0000000
#endif
    };
}

uint32_t arch_pci_read(pci_addr const &addr, uint32_t offset)
{
    PCI_ACCESS_ADDR = addr.pioofs(offset);
    uint32_t value = PCI_ACCESS_DATA;
    
    return value;
}


void arch_pci_write(pci_addr const &addr, uint32_t offset, uint32_t value)
{
    PCI_ACCESS_ADDR = addr.pioofs(offset);
    PCI_ACCESS_DATA = value;
}
