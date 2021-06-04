#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "dispi.h"
#include "debug.h"
#include "portio.h"
#include "../pci.h"
#include "assert.h"

#define LEGACY_ACCESS_ADDR  0xcf8
#define LEGACY_ACCESS_DATA  0xcfc

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