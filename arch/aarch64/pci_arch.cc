#include "arch/pci.h"
#include "debug.h"

static uint8_t * const ecam = (uint8_t *)0x4010000000;//0x3f000000;

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

static char volatile *plo11_base = (char volatile *)0x9000000;// len=0x1000

void arch_debug_char(uint8_t ch)
{
    *plo11_base = ch;
}

extern "C" void arch_halt()
{
    while (true)
        __asm__("wfi");
}
