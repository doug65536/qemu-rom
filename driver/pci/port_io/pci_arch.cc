#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "debug.h"
#include "machine/x86/portio_arch.h"
#include "arch/pci.h"
#include "assert.h"

#define LEGACY_ACCESS_ADDR  0xcf8
#define LEGACY_ACCESS_DATA  0xcfc

arch_mmio_range_t arch_mmio_range()
{
    return {
        0x80000000,
        0xe0000000
    };
}

uint32_t arch_pci_read(pci_addr const &addr, uint32_t offset)
{
    outd(LEGACY_ACCESS_ADDR, addr.pioofs(offset));
    uint32_t value = ind(LEGACY_ACCESS_DATA);
    
    return value;
}


void arch_pci_write(pci_addr const &addr, uint32_t offset, uint32_t value)
{
    outd(LEGACY_ACCESS_ADDR, addr.pioofs(offset));
    outd(LEGACY_ACCESS_DATA, value);
}
