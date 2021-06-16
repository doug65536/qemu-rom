#include "debug.h"
#include "arch/pci.h"

struct serial_device_t {
    uint64_t addr;
    bool is_mmio;
};

static constexpr size_t MAX_SERIAL_DEVICES = 16;
static serial_device_t serial_devices[MAX_SERIAL_DEVICES];
static size_t serial_device_count;
static pci_ready_node_t pci_serial_pci_ready_node;

static void pci_serial_pci_ready(void *)
{
    size_t index = size_t(-1);
    
    do {
        index = pci_enum_next_class_subclass_progif(index, 
                PCI_DEV_CLASS_COMM, PCI_SUBCLASS_COMM_16x50, -1, -1);
        
        uint64_t bar_base = pci_bar_get_base(index, 0);
        bool is_mmio = pci_bar_is_io(index, 0);
        
        if (serial_device_count < MAX_SERIAL_DEVICES)
            serial_devices[serial_device_count++] = { bar_base, is_mmio };
    } while (index != size_t(-1));
}

_constructor(10000)
static void pci_serial_init()
{
    pci_when_pci_ready(&pci_serial_pci_ready_node, 
            pci_serial_pci_ready, nullptr);
}

void arch_debug_char(uint8_t ch)
{
    if (serial_device_count > 0)
        if (serial_devices[0].is_mmio)
            *(uint8_t volatile *)(uintptr_t)serial_devices = ch;
}
