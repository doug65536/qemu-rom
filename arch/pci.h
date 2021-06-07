#pragma once
#include <stddef.h>
#include <stdint.h>

struct arch_mmio_range_t {
    uint64_t st;
    uint64_t en;
};

struct pci_addr {
    int bus = 0;
    int slot = 0;
    int func = 0;
    
    constexpr pci_addr(int bus, int slot, int func)
        : bus(bus), slot(slot), func(func) {}
    constexpr pci_addr() = default;
    constexpr pci_addr(pci_addr const&) = default;
    constexpr pci_addr &operator=(pci_addr const&) = default;
    constexpr pci_addr(pci_addr&&) = default;
    ~pci_addr() = default;

    constexpr uint32_t pioofs(uint32_t offset) const noexcept
    {
        return (1U << 31) |
            (bus << 16) |
            (slot << 11) |
            (func << 8) |
            (offset & -4);
    }
};

struct pci_config_hdr_t {
    // 0x00
    uint16_t vendor;
    uint16_t device;

    // 0x04
    uint16_t command;
    uint16_t status;

    // 0x08
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t dev_class;

    // 0x0C
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;

    // 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24
    uint32_t base_addr[6];

    // 0x28
    uint32_t cardbus_cis_ptr;

    // 0x2C
    uint16_t subsystem_vendor;
    uint16_t subsystem_id;

    // 0x30
    uint32_t expansion_rom_addr;

    // 0x34
    uint8_t capabilities_ptr;

    // 0x35howe
    uint8_t reserved[7];

    // 0x3C
    uint8_t irq_line;
    uint8_t irq_pin;
    uint8_t min_grant;
    uint8_t max_latency;
};

char const *pci_describe_device(uint8_t cls, uint8_t sc, uint8_t pif);

void set_bars(pci_addr const& addr,
    uint64_t &mm_next_place, uint64_t mm_limit, 
    uint32_t &io_next_place, uint64_t *bar_readback, uint32_t *alignments);

//
// Low level 32-bit only configuration space accesses

extern "C"
void arch_pci_write(pci_addr const& addr, uint32_t offset,
    uint32_t value);

extern "C"
uint32_t arch_pci_read(pci_addr const& addr, uint32_t offset);

extern "C"
arch_mmio_range_t arch_mmio_range();

//
// High level arbitrarily sized configuration space accesses

extern "C"
uint32_t pci_read(pci_addr const& addr, uint32_t size, uint32_t offset);

extern "C"
void pci_write(pci_addr const& addr, size_t size, uint32_t offset,
        uint32_t value);

#define PCI_DEV_CLASS_UNCLASSIFIED      0x00
#define PCI_DEV_CLASS_STORAGE           0x01
#define PCI_DEV_CLASS_NETWORK           0x02
#define PCI_DEV_CLASS_DISPLAY           0x03
#define PCI_DEV_CLASS_MULTIMEDIA        0x04
#define PCI_DEV_CLASS_MEMORY            0x05
#define PCI_DEV_CLASS_BRIDGE            0x06
#define PCI_DEV_CLASS_COMM              0x07
#define PCI_DEV_CLASS_SYSTEM            0x08
#define PCI_DEV_CLASS_INPUT             0x09
#define PCI_DEV_CLASS_DOCKING           0x0A
#define PCI_DEV_CLASS_PROCESSOR         0x0B
#define PCI_DEV_CLASS_SERIAL            0x0C
#define PCI_DEV_CLASS_WIRELESS          0x0D
#define PCI_DEV_CLASS_INTELLIGENT       0x0E
#define PCI_DEV_CLASS_SATELLITE         0x0F
#define PCI_DEV_CLASS_ENCRYPTION        0x10
#define PCI_DEV_CLASS_DSP               0x11
#define PCI_DEV_CLASS_ACCELERATOR       0x12
#define PCI_DEV_CLASS_INSTRUMENTATION   0x13
#define PCI_DEV_CLASS_COPROCESSOR       0x40
#define PCI_DEV_CLASS_UNASSIGNED        0xFF

// PCI_DEV_CLASS_UNCLASSIFIED
#define PCI_SUBCLASS_UNCLASSIFIED_OLD   0x00
#define PCI_SUBCLASS_UNCLASSIFIED_VGA   0x01

// PCI_DEV_CLASS_STORAGE
#define PCI_SUBCLASS_STORAGE_SCSI       0x00
#define PCI_SUBCLASS_STORAGE_IDE        0x01
#define PCI_SUBCLASS_STORAGE_FLOPPY     0x02
#define PCI_SUBCLASS_STORAGE_IPIBUS     0x03
#define PCI_SUBCLASS_STORAGE_RAID       0x04
#define PCI_SUBCLASS_STORAGE_ATA        0x05
#define PCI_SUBCLASS_STORAGE_SATA       0x06
#define PCI_SUBCLASS_STORAGE_SAS        0x07
#define PCI_SUBCLASS_STORAGE_NVM        0x08
#define PCI_SUBCLASS_STORAGE_MASS       0x80

// PCI_SUBCLASS_STORAGE_SATA
#define PCI_PROGIF_STORAGE_SATA_VEND    0x00
#define PCI_PROGIF_STORAGE_SATA_AHCI    0x01
#define PCI_PROGIF_STORAGE_SATA_SERIAL  0x02

// PCI_SUBCLASS_STORAGE_NVM
#define PCI_PROGIF_STORAGE_NVM_NVME		0x02

// PCI_DEV_CLASS_NETWORK
#define PCI_SUBCLASS_NETWORK_ETHERNET   0x00
#define PCI_SUBCLASS_NETWORK_TOKENRING  0x01
#define PCI_SUBCLASS_NETWORK_FDDI       0x02
#define PCI_SUBCLASS_NETWORK_ATM        0x03
#define PCI_SUBCLASS_NETWORK_ISDN       0x04
#define PCI_SUBCLASS_NETWORK_WFLIP      0x05
#define PCI_SUBCLASS_NETWORK_PICMGMC    0x06
#define PCI_SUBCLASS_NETWORK_OTHER      0x80

// PCI_DEV_CLASS_DISPLAY
#define PCI_SUBCLASS_DISPLAY_VGA        0x00
#define PCI_SUBCLASS_DISPLAY_XGA        0x01
#define PCI_SUBCLASS_DISPLAY_3D         0x02
#define PCI_SUBCLASS_DISPLAY_OTHER      0x80

// PCI_SUBCLASS_DISPLAY_VGA
#define PCI_PROGIF_DISPLAY_VGA_STD      0x00
#define PCI_PROGIF_DISPLAY_VGA_8514     0x01

// PCI_DEV_CLASS_MULTIMEDIA
#define PCI_SUBCLASS_MULTIMEDIA_VIDEO   0x00
#define PCI_SUBCLASS_MULTIMEDIA_AUDIO   0x01
#define PCI_SUBCLASS_MULTIMEDIA_TELEP   0x02
#define PCI_SUBCLASS_MULTIMEDIA_OTHER   0x80

// PCI_DEV_CLASS_MEMORY
#define PCI_SUBCLASS_MEMORY_RAM         0x00
#define PCI_SUBCLASS_MEMORY_FLASH       0x01
#define PCI_SUBCLASS_MEMORY_OTHER       0x80

// PCI_DEV_CLASS_BRIDGE
#define PCI_SUBCLASS_BRIDGE_HOST        0x00
#define PCI_SUBCLASS_BRIDGE_ISA         0x01
#define PCI_SUBCLASS_BRIDGE_EISA        0x02
#define PCI_SUBCLASS_BRIDGE_MCA         0x03
#define PCI_SUBCLASS_BRIDGE_PCI2PCI     0x04
#define PCI_SUBCLASS_BRIDGE_PCMCIA      0x05
#define PCI_SUBCLASS_BRIDGE_NUBUS       0x06
#define PCI_SUBCLASS_BRIDGE_CARDBUS     0x07
#define PCI_SUBCLASS_BRIDGE_RACEWAY     0x08
#define PCI_SUBCLASS_BRIDGE_SEMITP2P    0x09
#define PCI_SUBCLASS_BRIDGE_INFINITI    0x0A
#define PCI_SUBCLASS_BRIDGE_OTHER       0x80

// PCI_SUBCLASS_BRIDGE_PCI2PCI
#define PCI_SUBCLASS_PCI2PCI_NORMAL     0x00
#define PCI_SUBCLASS_PCI2PCI_SUBTRAC    0x01

// PCI_SUBCLASS_BRIDGE_SEMITP2P
#define PCI_PROGIF_BRIDGE_SEMITP2P_P    0x40
#define PCI_PROGIF_BRIDGE_SEMITP2P_S    0x80

// PCI_DEV_CLASS_COMM
#define PCI_SUBCLASS_COMM_16x50         0x00
#define PCI_SUBCLASS_COMM_PARALLEL      0x01
#define PCI_SUBCLASS_COMM_MULTIPORT     0x02
#define PCI_SUBCLASS_COMM_MODEM         0x03
#define PCI_SUBCLASS_COMM_GPIB          0x04
#define PCI_SUBCLASS_COMM_SMARTCARD     0x05
#define PCI_SUBCLASS_COMM_OTHER         0x80

// PCI_SUBCLASS_COMM_16x50
#define PCI_PROGIF_COMM_16x50_XT        0x00
#define PCI_PROGIF_COMM_16x50_16450     0x01
#define PCI_PROGIF_COMM_16x50_16550     0x02
#define PCI_PROGIF_COMM_16x50_16650     0x03
#define PCI_PROGIF_COMM_16x50_16750     0x04
#define PCI_PROGIF_COMM_16x50_16850     0x05
#define PCI_PROGIF_COMM_16x50_16960     0x06

// PCI_SUBCLASS_COMM_PARALLEL
#define PCI_PROGIF_COMM_PARALLEL_BASIC  0x00
#define PCI_PROGIF_COMM_PARALLEL_BIDIR  0x01
#define PCI_PROGIF_COMM_PARALLEL_ECP    0x02
#define PCI_PROGIF_COMM_PARALLEL_1284   0x03
#define PCI_PROGIF_COMM_PARALLEL_1284D  0xFE

// PCI_SUBCLASS_COMM_MODEM
#define PCI_PROGIF_COMM_MODEM_GENERIC   0x00
#define PCI_PROGIF_COMM_MODEM_HAYES_450 0x01
#define PCI_PROGIF_COMM_MODEM_HAYES_550 0x02
#define PCI_PROGIF_COMM_MODEM_HAYES_650 0x03
#define PCI_PROGIF_COMM_MODEM_HAYES_750 0x04

// PCI_DEV_CLASS_SYSTEM
#define PCI_SUBCLASS_SYSTEM_PIC         0x00
#define PCI_SUBCLASS_SYSTEM_DMA         0x01
#define PCI_SUBCLASS_SYSTEM_TIMER       0x02
#define PCI_SUBCLASS_SYSTEM_RTC         0x03
#define PCI_SUBCLASS_SYSTEM_HOTPLUG     0x04
#define PCI_SUBCLASS_SYSTEM_SDHOST      0x05
#define PCI_SUBCLASS_SYSTEM_OTHER       0x80

// PCI_SUBCLASS_SYSTEM_PIC
#define PCI_PROGIF_SYSTEM_PIC_8259      0x00
#define PCI_PROGIF_SYSTEM_PIC_ISA       0x01
#define PCI_PROGIF_SYSTEM_PIC_EISA      0x02
#define PCI_PROGIF_SYSTEM_PIC_IOAPIC    0x10
#define PCI_PROGIF_SYSTEM_PIC_IOXAPIC   0x20

// PCI_SUBCLASS_SYSTEM_DMA
#define PCI_PROGIF_SYSTEM_DMA_8237      0x00
#define PCI_PROGIF_SYSTEM_DMA_ISA       0x01
#define PCI_PROGIF_SYSTEM_DMA_EISA      0x02

// PCI_SUBCLASS_SYSTEM_TIMER
#define PCI_PROGIF_SYSTEM_TIMER_8254    0x00
#define PCI_PROGIF_SYSTEM_TIMER_ISA     0x01
#define PCI_PROGIF_SYSTEM_TIMER_EISA    0x02

// PCI_SUBCLASS_SYSTEM_RTC
#define PCI_PROGIF_SYSTEM_RTC_GENERIC   0x00
#define PCI_PROGIF_SYSTEM_RTC_ISA       0x01

// PCI_DEV_CLASS_INPUT
#define PCI_SUBCLASS_INPUT_KEYBOARD     0x00
#define PCI_SUBCLASS_INPUT_DIGIPEN      0x01
#define PCI_SUBCLASS_INPUT_MOUSE        0x02
#define PCI_SUBCLASS_INPUT_SCANNER      0x03
#define PCI_SUBCLASS_INPUT_GAME         0x04
#define PCI_SUBCLASS_INPUT_OTHER        0x80

// PCI_SUBCLASS_INPUT_GAME
#define PCI_PROGIF_INPUT_GAME_GENERIC   0x00
#define PCI_PROGIF_INPUT_GAME_STD       0x10

// PCI_DEV_CLASS_DOCKING
#define PCI_SUBCLASS_DOCKING_GENERIC    0x00
#define PCI_SUBCLASS_DOCKING_OTHER      0x80

// PCI_DEV_CLASS_PROCESSOR
#define PCI_SUBCLASS_PROCESSOR_386      0x00
#define PCI_SUBCLASS_PROCESSOR_486      0x01
#define PCI_SUBCLASS_PROCESSOR_PENTIUM  0x02
#define PCI_SUBCLASS_PROCESSOR_ALPHA    0x10
#define PCI_SUBCLASS_PROCESSOR_PPC      0x20
#define PCI_SUBCLASS_PROCESSOR_MIPS     0x30
#define PCI_SUBCLASS_PROCESSOR_COPROC   0x40

// PCI_DEV_CLASS_SERIAL
#define PCI_SUBCLASS_SERIAL_IEEE1394    0x00
#define PCI_SUBCLASS_SERIAL_ACCESS      0x01
#define PCI_SUBCLASS_SERIAL_SSA         0x02
#define PCI_SUBCLASS_SERIAL_USB         0x03
#define PCI_SUBCLASS_SERIAL_FIBRECHAN   0x04
#define PCI_SUBCLASS_SERIAL_SMBUS       0x05
#define PCI_SUBCLASS_SERIAL_INFINIBAND  0x06
#define PCI_SUBCLASS_SERIAL_IPMI        0x07
#define PCI_SUBCLASS_SERIAL_SERCOS      0x08
#define PCI_SUBCLASS_SERIAL_CANBUS      0x09

// PCI_SUBCLASS_SERIAL_IEEE1394
#define PCI_PROGIF_SERIAL_IEEE1394_FW   0x00
#define PCI_PROGIF_SERIAL_IEEE1394_FW   0x00

// PCI_SUBCLASS_SERIAL_USB
#define PCI_PROGIF_SERIAL_USB_UHCI      0x00
#define PCI_PROGIF_SERIAL_USB_OHCI      0x10
#define PCI_PROGIF_SERIAL_USB_EHCI      0x20
#define PCI_PROGIF_SERIAL_USB_XHCI      0x30
#define PCI_PROGIF_SERIAL_USB_UNSPEC    0x80
#define PCI_PROGIF_SERIAL_USB_USBDEV    0xFE

// PCI_SUBCLASS_SERIAL_IPMISMIC
#define PCI_PROGIF_SERIAL_IPMI_SMIC     0x00
#define PCI_PROGIF_SERIAL_IPMI_KEYBD    0x01
#define PCI_PROGIF_SERIAL_IPMI_BLOCK    0x02

// ?PCI_DEV_CLASS_WIRELESS
#define PCI_SUBCLASS_WIRELESS_IRDA      0x00
#define PCI_SUBCLASS_WIRELESS_IR        0x01
#define PCI_SUBCLASS_WIRELESS_RF        0x10
#define PCI_SUBCLASS_WIRELESS_BLUETOOTH 0x11
#define PCI_SUBCLASS_WIRELESS_BROADBAND 0x12
#define PCI_SUBCLASS_WIRELESS_ETH5GHz   0x20
#define PCI_SUBCLASS_WIRELESS_ETH2GHz   0x21
#define PCI_SUBCLASS_WIRELESS_OTHER     0x80

// PCI_DEV_CLASS_INTELLIGENT
#define PCI_SUBCLASS_INTELLIGENT_IO     0x00

// PCI_PROGIF_INTELLIGENT_IO
#define PCI_PROGIF_INTELLIGENT_IO_I2O   0x00
#define PCI_PROGIF_INTELLIGENT_IO_FIFO  0x01

// PCI_DEV_CLASS_SATELLITE
#define PCI_SUBCLASS_SATELLITE_TV       0x01
#define PCI_SUBCLASS_SATELLITE_AUDIO    0x02
#define PCI_SUBCLASS_SATELLITE_VOICE    0x03
#define PCI_SUBCLASS_SATELLITE_DATA     0x04

// ?PCI_DEV_CLASS_ENCRYPTION
#define PCI_SUBCLASS_ENCRYPTION_NET     0x00
#define PCI_SUBCLASS_ENCRYPTION_ENTAIN  0x10
#define PCI_SUBCLASS_ENCRYPTION_OTHER   0x80

// PCI_DEV_CLASS_DSP
#define PCI_SUBCLASS_DSP_DPIO           0x00
#define PCI_SUBCLASS_DSP_PERFCNT        0x01
#define PCI_SUBCLASS_DSP_COMMSYNC       0x10
#define PCI_SUBCLASS_DSP_MGMTCARD       0x20
#define PCI_SUBCLASS_DSP_OTHER          0x80

//
// Vendors

#define PCI_VENDOR_AMD      0x1022
#define PCI_VENDOR_INTEL    0x8086
#define PCI_VENDOR_ENSONIQ  0x1274

// config header command register/field bits
#define PCI_CMD_MSE 2
#define PCI_CMD_IOSE 1

// Low bits of BARs
#define PCI_BAR_FLAG_IO 1
#define PCI_BAR_FLAG_64 4
#define PCI_BAR_FLAG_PF 8

extern "C"
void pci_init();

extern "C"
size_t pci_device_count();

extern "C"
char const *pci_device_description(size_t index);

extern "C"
uint64_t pci_bar_get(size_t index, size_t bar);

extern "C"
uint64_t pci_bar_get_base(size_t index, size_t bar);

extern "C"
uint64_t pci_bar_size(size_t index, size_t bar);

extern "C"
bool pci_bar_is_io(size_t index, size_t bar);

extern "C"
bool pci_bar_is_64(size_t index, size_t bar);

extern "C"
bool pci_bar_is_prefetchable(size_t index, size_t bar);

extern "C"
bool pci_space_enable(size_t index, bool memory_space, bool io_space);

extern "C"
size_t pci_enum_next_vendor_device(size_t after, 
    uint16_t vendor, uint16_t device);

extern "C"
size_t pci_enum_next_class_subclass_progif(size_t after, 
    int dev_class, int subclass, int progif, int revision);
