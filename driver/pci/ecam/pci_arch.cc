#include "arch/pci.h"
#include "debug.h"
#include "arch/halt.h"
#include "config.h"

uint8_t volatile *ecam = (uint8_t*)ECAM_ST;

static size_t ecamofs(pci_addr const& addr, int bus_adj, size_t offset)
{
    return ((addr.bus - bus_adj) << 20) +
            (addr.slot << 15) +
            (addr.func << 12) +
            unsigned(offset & -4);
}

arch_mmio_range_t arch_mmio_range()
{
    return {
        0x10000000,
        0x3eff0000
    };
}

// Only do the raw 32 bit access, nothing more
uint32_t arch_pci_read(pci_addr const &addr, uint32_t offset)
{
    size_t ecam_offset = ecamofs(addr, 0, offset);
    
    uint32_t volatile *value_ptr = (uint32_t *)(ecam + ecam_offset);
    
    uint32_t value = *value_ptr;
    
    return value;
}

void arch_pci_write(pci_addr const &addr, uint32_t offset, uint32_t value)
{
    size_t ecam_offset = ecamofs(addr, 0, offset);
    
    uint32_t volatile *value_ptr = (uint32_t *)(ecam + ecam_offset);
    
    *value_ptr = value;
}
