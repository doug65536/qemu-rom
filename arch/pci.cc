#include <stdint.h>
#include "pci.h"
#include "debug.h"
#include "assert.h"

static pci_ready_node_t *pci_ready_node_first;
static bool pci_ready_already;

uint32_t pci_read(pci_addr const& addr, uint32_t size, uint32_t offset)
{
    // Fastpath easy stuff
    if (size == sizeof(uint32_t)) {
        assert((offset & -4) == offset);
        return arch_pci_read(addr, offset);
    }

    uint32_t word_offset = offset & (sizeof(uint32_t) - 1);

    offset &= -4;

    uint32_t value = arch_pci_read(addr, offset);

    uint32_t shift = word_offset * 8;

    value >>= shift;

    // Naturally aligned
    assert((word_offset & -size) == word_offset);

    // No spanning allowed
    assert(((word_offset + (size - 1)) & -size) == word_offset);

    switch (size) {
    case 1:
        value &= 0xFFU;
        break;

    case 2:
        value &= 0xFFFFU;
        break;

    }

    return value;
}

void pci_write(pci_addr const& addr, size_t size, uint32_t offset,
        uint32_t value)
{
    // Fastpath easy stuff
    if (size == sizeof(uint32_t)) {
        assert((offset & -4) == offset);
        return arch_pci_write(addr, offset, value);
    }
    
    // Naturally aligned
    assert((offset & -size) == offset);

    // No spanning allowed
    assert(((offset + (size - 1)) & -size) == offset);

    size_t word_offset = offset & 3;

    offset &= -4;

    uint32_t old_value = 0;

    if (size != sizeof(uint32_t))
        old_value = arch_pci_read(addr, offset);

    size_t shift = word_offset * 8;

    value <<= shift;

    switch (size) {
    case 1:
        old_value &= ~(0xFFU << shift);
        break;

    case 2:
        old_value &= ~(0xFFFFU << shift);
        break;

    case 4:
        old_value = 0;
        break;

    }

    value |= old_value;

    arch_pci_write(addr, offset, value);
}

void set_bars(pci_addr const& addr, uint64_t &mm_next_place, uint64_t mm_limit,
        uint32_t &io_next_place, uint64_t *bar_readback, uint32_t *alignments)
{
    uint32_t bars[5];
    size_t base_addr_ofs = offsetof(pci_config_hdr_t, base_addr);
    
    bool is64 = false;

    for (size_t i = 0; i < 5; i = (i + 1) + is64) {
        uint32_t bar_ofs = base_addr_ofs + (sizeof(*bars) * i);
        
        printdbg("bar_ofs %zu\n", (size_t)bar_ofs);

        uint32_t bar = pci_read(addr, sizeof(uint32_t), bar_ofs);
        
        uint32_t next_bar = 0;
        
        is64 = (bar & 6) == 4;
        bool isio = bar & 1;
        //cachability is not important bool ispf = bar & 8;
        
        if (bar < 5 && is64)
            next_bar = arch_pci_read(addr, bar_ofs + sizeof(uint32_t));

        arch_pci_write(addr, bar_ofs, -16U);
        
        if (is64) {
            arch_pci_write(addr, bar_ofs + sizeof(uint32_t), -1U);
        }

        uint64_t readback = pci_read(addr, sizeof(uint32_t), bar_ofs);
        
        if (is64) {
            readback |= (uint64_t)pci_read(addr, sizeof(uint32_t), 
                    bar_ofs + sizeof(uint32_t)) << 32;
        }

        if (!readback) {
            arch_pci_write(addr, bar_ofs, bar);
            arch_pci_write(addr, bar_ofs + sizeof(uint32_t), next_bar);
            continue;
        }

        uint32_t alignment = -(readback & -16);
        
        if (alignments)
            alignments[i] = alignment;

        uint64_t base;
        if (!isio) {
            // Place as close as possible below ROM image
            base = (mm_next_place & -alignment) - alignment;
            assert(base >= mm_limit);
            if (base < mm_limit)
                continue;
            mm_next_place = base;
        } else {
            base = (io_next_place & -alignment) - alignment;
            io_next_place = base;
        }
        
        
        pci_write(addr, sizeof(uint32_t), 
                bar_ofs, uint32_t(base & 0xFFFFFFFF));
        
        if (is64)
            pci_write(addr, sizeof(uint32_t), bar_ofs + sizeof(uint32_t), 0);
        
        
        bar_readback[i] = base;
        
        readback = pci_read(addr, sizeof(uint32_t), bar_ofs);
        
        if (is64) {
            if ((i + 1) < 5)
                bar_readback[i + 1] = 0;
            
            readback |= (uint64_t)pci_read(addr, sizeof(uint32_t), 
                    bar_ofs + sizeof(uint32_t)) << 32;
        }
        
        bar_readback[i] = readback;
        
        uint32_t command = pci_read(addr, sizeof(uint16_t),
                offsetof(pci_config_hdr_t, command));
        
        if (!isio)
            command |= PCI_CMD_MSE;
        else
            command |= PCI_CMD_IOSE;

        pci_write(addr, sizeof(uint16_t),
                offsetof(pci_config_hdr_t, command), command);
    }
}

struct pci_device_summary_t {
    uint8_t bus = 0;            // < 16
    uint8_t slot = 0;           // < 32
    uint8_t func = 0;           // < 8
    uint8_t reserved = 0;
    
    uint16_t vendor = 0;
    uint16_t device = 0;
    
    uint8_t dev_class = 0;
    uint8_t subclass = 0;
    uint8_t prog_if = 0;
    uint8_t revision = 0;
    
    uint64_t bars[5] = {};
    uint64_t bar_sizes[5] = {};
    bool is_io[5] = {};
    bool is_pf[5] = {};
    uint8_t header_type = 0;
    bool initialized = false;
    
    constexpr pci_device_summary_t() = default;
    
    constexpr pci_device_summary_t(
        uint8_t bus, uint8_t slot, uint8_t func,
        uint16_t vendor, uint16_t device,
        
        uint8_t dev_class, uint8_t subclass, 
        uint8_t prog_if, uint8_t revision,
        
        uint8_t header_type)
        : bus(bus)
        , slot(slot)
        , func(func)
        , reserved(0)
        , vendor(vendor)
        , device(device)
        , dev_class(dev_class)
        , subclass(subclass)
        , prog_if(prog_if)
        , revision(revision)
        , bars{}
        , is_io{}
        , is_pf{}
        , header_type(header_type)
        , initialized(false)
    {    
    }
};

static constexpr size_t device_limit = 128;
static pci_device_summary_t devices[device_limit];
size_t device_count;

void pci_init()
{
    int bus_todo[256];
    int *todo_end = bus_todo + 256;
    int *todo_ptr = todo_end;

    *--todo_ptr = 0;
    
    arch_mmio_range_t mmio_range = arch_mmio_range();

    uint64_t mm_base = mmio_range.en;
    uint32_t io_base = 0xf000;

    while (todo_ptr < todo_end) {
        int bus = *todo_ptr++;

        for (int slot = 0; slot < 32; ++slot) {
            int func_count = 1;

            for (int func = 0; func < func_count; ++func) {
                pci_addr addr{bus, slot, func};
                
                printdbg("Checking %d:%d:%d\n", bus, slot, func);

                uint8_t header_type = pci_read(addr, sizeof(uint8_t),
                            offsetof(pci_config_hdr_t, header_type));
                
                if (header_type == 0xFF)
                    break;

                if (func == 0) {
                    bool multifunction = header_type & 0x80;

                    // Extend function loop if header type says multifunction
                    if (multifunction)
                        func_count = 8;
                }

                uint16_t vendor = pci_read(addr, sizeof(uint16_t),
                        offsetof(pci_config_hdr_t, vendor));

                uint16_t device = pci_read(addr, sizeof(uint16_t),
                        offsetof(pci_config_hdr_t, device));

                uint8_t dev_class = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, dev_class));

                if (dev_class == 0xFF)
                    break;

                uint8_t subclass = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, subclass));

                uint8_t prog_if = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, prog_if));

                uint8_t revision = pci_read(addr, sizeof(uint8_t),
                        offsetof(pci_config_hdr_t, revision));
                
                pci_device_summary_t &dev = devices[device_count++];
                
                dev = pci_device_summary_t(
                        bus, slot, func, 
                        vendor, device, 
                        dev_class, subclass, prog_if, revision, 
                        header_type);
                
                char const *description = pci_describe_device(
                        dev_class, subclass, prog_if);
                
                printdbg("class=%x subclass=%x %s\n",
                        dev_class, subclass, description);

                if (dev_class == PCI_DEV_CLASS_BRIDGE &&
                        subclass == PCI_SUBCLASS_BRIDGE_PCI2PCI) {
                    uint8_t secondary_bus = pci_read(
                            addr, sizeof(uint8_t),
                            offsetof(pci_config_hdr_t, base_addr[2]) + 1);

                    if (todo_ptr > bus_todo)
                        *--todo_ptr = secondary_bus;
                }
                
                if (!(header_type & 0x7F)) {
                    uint32_t bar_alignments[5];
                    uint64_t bar_readback[5];
                    set_bars(addr, mm_base, mmio_range.st, io_base, 
                            bar_readback, bar_alignments);
                    for (size_t i = 0; i < 5; ++i) {
                        dev.bar_sizes[i] = bar_alignments[i];
                        dev.bars[i] = bar_readback[i];
                        dev.is_io[i] = bar_readback[i] & PCI_BAR_FLAG_IO;
                        dev.is_pf[i] = bar_readback[i] & PCI_BAR_FLAG_PF;
                    }
                }
            }
        }
    }
    
    pci_notify_pci_ready();
}

char const *pci_describe_device(uint8_t cls, uint8_t sc, uint8_t pif)
{
    switch (cls) {
    case PCI_DEV_CLASS_UNCLASSIFIED:
        switch (sc) {
        case PCI_SUBCLASS_UNCLASSIFIED_OLD:
            return "Unclassified/Old";
        case PCI_SUBCLASS_UNCLASSIFIED_VGA:
            return "Unclassified/VGA";
        default:
            return "Unclassified/Unknown";
        }
    case PCI_DEV_CLASS_STORAGE:
        switch (sc) {
        case PCI_SUBCLASS_STORAGE_SCSI:
            return "Storage/SCSI";
        case PCI_SUBCLASS_STORAGE_IDE:
            return "Storage/IDE";
        case PCI_SUBCLASS_STORAGE_FLOPPY:
            return "Storage/Floppy";
        case PCI_SUBCLASS_STORAGE_IPIBUS:
            return "Storage/IPIBus";
        case PCI_SUBCLASS_STORAGE_RAID:
            return "Storage/RAID";
        case PCI_SUBCLASS_STORAGE_ATA:
            return "Storage/ATA";
        case PCI_SUBCLASS_STORAGE_SATA:
            switch (pif) {
            case PCI_PROGIF_STORAGE_SATA_VEND:
                return "Storage/SATA/Vendor Specific";
            case PCI_PROGIF_STORAGE_SATA_AHCI:
                return "Storage/SATA/AHCI";
            case PCI_PROGIF_STORAGE_SATA_SERIAL:
                return "Storage/SATA/Serial";
            default:
                return "Storage/SATA/Unknown";
            }
        case PCI_SUBCLASS_STORAGE_SAS:
            return "Storage/SAS";
        case PCI_SUBCLASS_STORAGE_NVM:
            switch (pif) {
            case PCI_PROGIF_STORAGE_NVM_NVME:
                return "Storage/NVM/NVMe";
            default:
                return "Storage/NVM/Unknown";
            }
        case PCI_SUBCLASS_STORAGE_MASS:
            return "Storage/Mass";
        default:
            return "Storage/Unknown";
        }
    case PCI_DEV_CLASS_NETWORK:
        switch (sc) {
        case PCI_SUBCLASS_NETWORK_ETHERNET:
            return "Network/Ethernet";
        case PCI_SUBCLASS_NETWORK_TOKENRING:
            return "Network/TokenRing";
        case PCI_SUBCLASS_NETWORK_FDDI:
            return "Network/FDDI";
        case PCI_SUBCLASS_NETWORK_ATM:
            return "Network/ATM";
        case PCI_SUBCLASS_NETWORK_ISDN:
            return "Network/IDSN";
        case PCI_SUBCLASS_NETWORK_WFLIP:
            return "Network/WFLIP";
        case PCI_SUBCLASS_NETWORK_PICMGMC:
            return "Network/PICMGMC";
        case PCI_SUBCLASS_NETWORK_OTHER:
            return "Network/Other";
        default:
            return "Network/Unknown";
        }
    case PCI_DEV_CLASS_DISPLAY:
        switch (sc) {
        case PCI_SUBCLASS_DISPLAY_VGA:
            switch (pif) {
            case PCI_PROGIF_DISPLAY_VGA_STD:
                return "Display/VGA/Standard";
            case PCI_PROGIF_DISPLAY_VGA_8514:
                return "Display/VGA/8514";
            default:
                return "Display/VGA/Unknown";
            }
        case PCI_SUBCLASS_DISPLAY_XGA:
            return "Display/XGA";
        case PCI_SUBCLASS_DISPLAY_3D:
            return "Display/3D";
        case PCI_SUBCLASS_DISPLAY_OTHER:
            return "Display/Other";
        default:
            return "Display/Unknown";
        }
    case PCI_DEV_CLASS_MULTIMEDIA:
        switch (sc) {
        case PCI_SUBCLASS_MULTIMEDIA_VIDEO:
            return "Multimedia/Video";
        case PCI_SUBCLASS_MULTIMEDIA_AUDIO:
            return "Multimedia/Audio";
        case PCI_SUBCLASS_MULTIMEDIA_TELEP:
            return "Multimedia/Telephony";
        case PCI_SUBCLASS_MULTIMEDIA_OTHER:
            return "Multimedia/Other";
        default:
            return "Multimedia/Unknown";
        }
    case PCI_DEV_CLASS_MEMORY:
        switch (sc) {
        case PCI_SUBCLASS_MEMORY_RAM:
            return "Memory/RAM";
        case PCI_SUBCLASS_MEMORY_FLASH:
            return "Memory/Flash";
        case PCI_SUBCLASS_MEMORY_OTHER:
            return "Memory/Other";
        default:
            return "Memory/Unknown";
        }
    case PCI_DEV_CLASS_BRIDGE:
        switch (sc) {
        case PCI_SUBCLASS_BRIDGE_HOST:
            return "Bridge/Host";
        case PCI_SUBCLASS_BRIDGE_ISA:
            return "Bridge/ISA";
        case PCI_SUBCLASS_BRIDGE_EISA:
            return "Bridge/EISA";
        case PCI_SUBCLASS_BRIDGE_MCA:
            return "Bridge/MCA";
        case PCI_SUBCLASS_BRIDGE_PCI2PCI:
            switch (pif) {
            case PCI_SUBCLASS_PCI2PCI_NORMAL:
                return "Bridge/PCI2PCI/Normal";
            case PCI_SUBCLASS_PCI2PCI_SUBTRAC:
                return "Bridge/PCI2PCI/Subtractive";
            default:
                return "Bridge/PCI2PCI/Unknown";
            }
        case PCI_SUBCLASS_BRIDGE_PCMCIA:
            return "Bridge/PCMCIA";
        case PCI_SUBCLASS_BRIDGE_NUBUS:
            return "Bridge/NuBus";
        case PCI_SUBCLASS_BRIDGE_CARDBUS:
            return "Bridge/CardBus";
        case PCI_SUBCLASS_BRIDGE_RACEWAY:
            return "Bridge/RaceWay";
        case PCI_SUBCLASS_BRIDGE_SEMITP2P:
            switch (pif) {
            case PCI_PROGIF_BRIDGE_SEMITP2P_P:
                return "Bridge/SEMITP2P/P";
            case PCI_PROGIF_BRIDGE_SEMITP2P_S:
                return "Bridge/SEMITP2P/S";
            default:
                return "Bridge/SEMITP2P/Unknown";
            }
        case PCI_SUBCLASS_BRIDGE_INFINITI:
            return "Bridge/Infiniti";
        case PCI_SUBCLASS_BRIDGE_OTHER:
            return "Bridge/Other";
        default:
            return "Bridge/Unknown";
        }
    case PCI_DEV_CLASS_COMM:
        switch (sc) {
        case PCI_SUBCLASS_COMM_16x50:
            switch (pif) {
            case PCI_PROGIF_COMM_16x50_XT:
                return "Comm/16x50/XT";
            case PCI_PROGIF_COMM_16x50_16450:
                return "Comm/16x50/16450";
            case PCI_PROGIF_COMM_16x50_16550:
                return "Comm/16x50/16550";
            case PCI_PROGIF_COMM_16x50_16650:
                return "Comm/16x50/16650";
            case PCI_PROGIF_COMM_16x50_16750:
                return "Comm/16x50/16750";
            case PCI_PROGIF_COMM_16x50_16850:
                return "Comm/16x50/16850";
            case PCI_PROGIF_COMM_16x50_16960:
                return "Comm/16x50/16950";
            default:
                return "Comm/16x50/Unknown";
            }
        case PCI_SUBCLASS_COMM_PARALLEL:
            // PCI_SUBCLASS_COMM_PARALLEL
            switch (pif) {
            case PCI_PROGIF_COMM_PARALLEL_BASIC:
                return "Comm/Parallel/Basic";
            case PCI_PROGIF_COMM_PARALLEL_BIDIR:
                return "Comm/Parallel/Bidirectional";
            case PCI_PROGIF_COMM_PARALLEL_ECP:
                return "Comm/Parallel/ECP";
            case PCI_PROGIF_COMM_PARALLEL_1284:
                return "Comm/Parallel/1284";
            case PCI_PROGIF_COMM_PARALLEL_1284D:
                return "Comm/Parallel/1284D";
            default:
                return "Comm/Parallel/Unknown";
            }
        case PCI_SUBCLASS_COMM_MULTIPORT:
            return "Comm/Multiport";
        case PCI_SUBCLASS_COMM_MODEM:
            switch (pif) {
            case PCI_PROGIF_COMM_MODEM_GENERIC:
                return "Comm/Modem/Generic";
            case PCI_PROGIF_COMM_MODEM_HAYES_450:
                return "Comm/Modem/Hayes_450";
            case PCI_PROGIF_COMM_MODEM_HAYES_550:
                return "Comm/Modem/Hayes_550";
            case PCI_PROGIF_COMM_MODEM_HAYES_650:
                return "Comm/Modem/Hayes_650";
            case PCI_PROGIF_COMM_MODEM_HAYES_750:
                return "Comm/Modem/Hayes_750";
            default:
                return "Comm/Modem/Unknown";
            }
        case PCI_SUBCLASS_COMM_GPIB:
            return "Comm/GPIB";
        case PCI_SUBCLASS_COMM_SMARTCARD:
            return "Comm/SmartCard";
        case PCI_SUBCLASS_COMM_OTHER:
            return "Comm/Other";
        default:
            return "Comm/Unknown";
        }
    case PCI_DEV_CLASS_SYSTEM:
        switch (sc) {
        case PCI_SUBCLASS_SYSTEM_PIC:
            switch (pif) {
            case PCI_PROGIF_SYSTEM_PIC_8259:
                return "System/PIC/8259";
            case PCI_PROGIF_SYSTEM_PIC_ISA:
                return "System/PIC/ISA";
            case PCI_PROGIF_SYSTEM_PIC_EISA:
                return "System/PIC/EISA";
            case PCI_PROGIF_SYSTEM_PIC_IOAPIC:
                return "System/PIC/IOAPIC";
            case PCI_PROGIF_SYSTEM_PIC_IOXAPIC:
                return "System/PIC/IOXAPIC";
            default:
                return "System/PIC/Unknown";
            }
        case PCI_SUBCLASS_SYSTEM_DMA:
            switch (pif) {
            case PCI_PROGIF_SYSTEM_DMA_8237:
                return "System/DMA/8237";
            case PCI_PROGIF_SYSTEM_DMA_ISA:
                return "System/DMA/ISA";
            case PCI_PROGIF_SYSTEM_DMA_EISA:
                return "System/DMA/EISA";
            default:
                return "System/DMA/Unknown";
            }
        case PCI_SUBCLASS_SYSTEM_TIMER:
            switch (pif) {
            case PCI_PROGIF_SYSTEM_TIMER_8254:
                return "System/Timer/8254";
            case PCI_PROGIF_SYSTEM_TIMER_ISA:
                return "System/Timer/ISA";
            case PCI_PROGIF_SYSTEM_TIMER_EISA:
                return "System/Timer/EISA";
            default:
                return "System/Timer/Unknown";
            }
        case PCI_SUBCLASS_SYSTEM_RTC:
            switch (pif) {
            case PCI_PROGIF_SYSTEM_RTC_GENERIC:
                return "System/RTC/Generic";
            case PCI_PROGIF_SYSTEM_RTC_ISA:
                return "System/RTC/ISA";
            default:
                return "System/RTC/Unknown";
            }
        case PCI_SUBCLASS_SYSTEM_HOTPLUG:
            return "System/Hotplug";
        case PCI_SUBCLASS_SYSTEM_SDHOST:
            return "System/SDHost";
        case PCI_SUBCLASS_SYSTEM_OTHER:
            return "System/Other";
        default:
            return "System/Unknown";
        }
    case PCI_DEV_CLASS_INPUT:
        switch (sc) {
        case PCI_SUBCLASS_INPUT_KEYBOARD:
            return "Input/Keyboard";
        case PCI_SUBCLASS_INPUT_DIGIPEN:
            return "Input/DigiPen";
        case PCI_SUBCLASS_INPUT_MOUSE:
            return "Input/Mouse";
        case PCI_SUBCLASS_INPUT_SCANNER:
            return "Input/Scanner";
        case PCI_SUBCLASS_INPUT_GAME:
            switch (pif) {
            case PCI_PROGIF_INPUT_GAME_GENERIC:
                return "Input/Game/Generic";
            case PCI_PROGIF_INPUT_GAME_STD:
                return "Input/Game/Standard";
            default:
                return "Input/Game/Unknown";
            }
        case PCI_SUBCLASS_INPUT_OTHER:
            return "Input/Other";
        default:
            return "Input/Unknown";
        }
    case PCI_DEV_CLASS_DOCKING:
        switch (sc) {
        case PCI_SUBCLASS_DOCKING_GENERIC:
            return "Docking/Generic";
        case PCI_SUBCLASS_DOCKING_OTHER:
            return "Docking/Other";
        default:
            return "Docking/Unknown";
        }
    case PCI_DEV_CLASS_PROCESSOR:
        switch (sc) {
        case PCI_SUBCLASS_PROCESSOR_386:
            return "Processor/386";
        case PCI_SUBCLASS_PROCESSOR_486:
            return "Processor/486";
        case PCI_SUBCLASS_PROCESSOR_PENTIUM:
            return "Processor/Pentium";
        case PCI_SUBCLASS_PROCESSOR_ALPHA:
            return "Processor/Alpha";
        case PCI_SUBCLASS_PROCESSOR_PPC:
            return "Processor/PPC";
        case PCI_SUBCLASS_PROCESSOR_MIPS:
            return "Processor/MIPS";
        case PCI_SUBCLASS_PROCESSOR_COPROC:
            return "Processor/Coprocessor";
        default:
            return "Processor/Unknown";
        }
    case PCI_DEV_CLASS_SERIAL:
        switch (sc) {
        case PCI_SUBCLASS_SERIAL_IEEE1394:
            switch (pif) {
            case PCI_PROGIF_SERIAL_IEEE1394_FW:
                return "Serial/IEEE1394/FW";
            default:
                return "Serial/IEEE1394/Unknown";
            }
        case PCI_SUBCLASS_SERIAL_ACCESS:
        case PCI_SUBCLASS_SERIAL_SSA:
        case PCI_SUBCLASS_SERIAL_USB:
            switch (pif) {
            case PCI_PROGIF_SERIAL_USB_UHCI:
                return "Serial/USB/UHCI";
            case PCI_PROGIF_SERIAL_USB_OHCI:
                return "Serial/USB/OHCI";
            case PCI_PROGIF_SERIAL_USB_EHCI:
                return "Serial/USB/EHCI";
            case PCI_PROGIF_SERIAL_USB_XHCI:
                return "Serial/USB/XHCI";
            case PCI_PROGIF_SERIAL_USB_UNSPEC:
                return "Serial/USB/Unspecified";
            case PCI_PROGIF_SERIAL_USB_USBDEV:
                return "Serial/USB/USBDev";
            default:
                return "Serial/USB/Unknown";
            }
        case PCI_SUBCLASS_SERIAL_FIBRECHAN:
            return "Serial/FibreChannel";
        case PCI_SUBCLASS_SERIAL_SMBUS:
            return "Serial/SMBus";
        case PCI_SUBCLASS_SERIAL_INFINIBAND:
            return "Serial/InfiniBand";
        case PCI_SUBCLASS_SERIAL_IPMI:
            switch (pif) {
            case PCI_PROGIF_SERIAL_IPMI_SMIC:
                return "Serial/IPMI/SMIC";
            case PCI_PROGIF_SERIAL_IPMI_KEYBD:
                return "Serial/IPMI/Keyboard";
            case PCI_PROGIF_SERIAL_IPMI_BLOCK:
                return "Serial/IPMI/Block";
            default:
                return "Serial/IPMI/Unknown";
            }
        case PCI_SUBCLASS_SERIAL_SERCOS:
            return "Serial/SerCos";
        case PCI_SUBCLASS_SERIAL_CANBUS:
            return "Serial/CanBus";
        default:
            return "Serial/Unknown";
        }
    case PCI_DEV_CLASS_WIRELESS:
        switch (sc) {
        case PCI_SUBCLASS_WIRELESS_IRDA:
            return "Wireless/IRDA";
        case PCI_SUBCLASS_WIRELESS_IR:
            return "Wireless/IR";
        case PCI_SUBCLASS_WIRELESS_RF:
            return "Wireless/RF";
        case PCI_SUBCLASS_WIRELESS_BLUETOOTH:
            return "Wireless/BlueTooth";
        case PCI_SUBCLASS_WIRELESS_BROADBAND:
            return "Wireless/Broadband";
        case PCI_SUBCLASS_WIRELESS_ETH5GHz:
            return "Wireless/Ethernet5GHz";
        case PCI_SUBCLASS_WIRELESS_ETH2GHz:
            return "Wireless/Ethernet2.4GHz";
        case PCI_SUBCLASS_WIRELESS_OTHER:
            return "Wireless/Other";
        default:
            return "Wireless/Unknown";
        }
    case PCI_DEV_CLASS_INTELLIGENT:
        switch (sc) {
        case PCI_SUBCLASS_INTELLIGENT_IO:
            switch (pif) {
            case PCI_PROGIF_INTELLIGENT_IO_I2O:
                return "Intelligent/IO/I2O";
            case PCI_PROGIF_INTELLIGENT_IO_FIFO:
                return "Intelligent/IO/FIFO";
            default:
                return "Intelligent/IO/Unknown";
            }
        default:
            return "Intelligent/Unknown";
        }
    case PCI_DEV_CLASS_SATELLITE:
        switch (sc) {
        case PCI_SUBCLASS_SATELLITE_TV:
            return "Satellite/TV";
        case PCI_SUBCLASS_SATELLITE_AUDIO:
            return "Satellite/Audio";
        case PCI_SUBCLASS_SATELLITE_VOICE:
            return "Satellite/Voice";
        case PCI_SUBCLASS_SATELLITE_DATA:
            return "Satellite/Data";
        default:
            return "Satellite/Unknown";
        }
    case PCI_DEV_CLASS_ENCRYPTION:
        switch (sc) {
        case PCI_SUBCLASS_ENCRYPTION_NET:
            return "Encryption/Net";
        case PCI_SUBCLASS_ENCRYPTION_ENTAIN:
            return "Encryption/Entain";
        case PCI_SUBCLASS_ENCRYPTION_OTHER:
            return "Encryption/Other";
        default:
            return "Encryption/Unknown";
        }
    case PCI_DEV_CLASS_DSP:
        switch (sc) {
        case PCI_SUBCLASS_DSP_DPIO:
            return "DSP/DPIO";
        case PCI_SUBCLASS_DSP_PERFCNT:
            return "DSP/PerfCount";
        case PCI_SUBCLASS_DSP_COMMSYNC:
            return "DSP/CommSync";
        case PCI_SUBCLASS_DSP_MGMTCARD:
            return "DSP/ManagementCard";
        case PCI_SUBCLASS_DSP_OTHER:
            return "DSP/Other";
        default:
            return "DSP/Unknown";
        }
    case PCI_DEV_CLASS_ACCELERATOR:
        switch (sc) {
        default:
            return "Accelerator/Unknown";
        }
    case PCI_DEV_CLASS_INSTRUMENTATION:
        switch (sc) {
        default:
            return "Instrumentation/Unknown";
        }
    case PCI_DEV_CLASS_COPROCESSOR:
        switch (sc) {
        default:
            return "Coprocessor/Unknown";
        }
    case PCI_DEV_CLASS_UNASSIGNED:
        switch (sc) {
        default:
            return "Unassigned/Unknown";
        }
    default:
        return "Unknown";
    }
}

const char *pci_device_description(size_t index)
{
    if (index >= device_count)
        return nullptr;
    
    pci_device_summary_t const &dev = devices[index];
    
    return pci_describe_device(dev.dev_class, dev.subclass, dev.prog_if);
}

static bool pci_raw_bar_is_64(uint64_t bar)
{
    return bar & PCI_BAR_FLAG_64;
}

static bool pci_is_valid_index_bar(size_t index, size_t bar)
{
    // If reading out of range device or bar, not valid
    if (index >= device_count || bar >= 5)
        return false;
    
    pci_device_summary_t const &dev = devices[index];
    
    // If reading high half of a 64 bit bar directly, not valid
    if (bar > 0 && pci_raw_bar_is_64(dev.bars[bar - 1]))
        return false;
    
    return true;
}

uint64_t pci_bar_get(size_t index, size_t bar)
{
    if (!pci_is_valid_index_bar(index, bar))
        return false;
    
    pci_device_summary_t const &dev = devices[index];
    
    // Read two halves if it is 64 bit
    if (pci_raw_bar_is_64(dev.bars[bar]) && bar < 4)
        return dev.bars[bar] | (dev.bars[bar+1] << 32);
    
    // Read just one 32 bit one
    return dev.bars[bar];
}

uint64_t pci_bar_get_base(size_t index, size_t bar)
{
    return pci_bar_get(index, bar) & -16;
}

bool pci_bar_is_io(size_t index, size_t bar)
{
    if (!pci_is_valid_index_bar(index, bar))
        return false;
    
    pci_device_summary_t const &dev = devices[index];
    
    // Not sensible to read high half of 64 bit register directory
    if (bar > 0 && pci_raw_bar_is_64(dev.bars[bar-1]))
        return false;
    
    return dev.bars[bar] & PCI_BAR_FLAG_IO;
}

bool pci_bar_is_64(size_t index, size_t bar)
{
    if (!pci_is_valid_index_bar(index, bar))
        return false;
    
    pci_device_summary_t const &dev = devices[index];
    
    return dev.bars[bar] & PCI_BAR_FLAG_64;
}

bool pci_bar_is_prefetchable(size_t index, size_t bar)
{
    if (!pci_is_valid_index_bar(index, bar))
        return 0;
    
    pci_device_summary_t const &dev = devices[index];
    
    return dev.bars[bar] & PCI_BAR_FLAG_PF;
}

bool pci_space_enable(size_t index, bool memory_space, bool io_space)
{
    if (index >= device_count)
        return false;
    
    pci_device_summary_t const &dev = devices[index];
    
    uint16_t command = pci_read(pci_addr(dev.bus, dev.slot, dev.func), 
            sizeof(uint16_t), offsetof(pci_config_hdr_t, command));
    
    command = (command & ~(PCI_CMD_MSE | PCI_CMD_IOSE)) |
        (-memory_space & PCI_CMD_MSE) |
        (-io_space & PCI_CMD_IOSE);
    
    pci_write(pci_addr(dev.bus, dev.slot, dev.func), 
            sizeof(uint16_t), offsetof(pci_config_hdr_t, command), 
            command);
    
    return true;
}

size_t pci_enum_next_vendor_device(size_t after, 
        uint16_t vendor, uint16_t device)
{
    size_t index = after;
    while (++index < device_count) {
        pci_device_summary_t const& dev = devices[index];
        
        if (dev.vendor == vendor && dev.device == device)
            return index;
    }
    
    return size_t(-1);
}

size_t pci_enum_next_class_subclass_progif(size_t after, 
        int dev_class, int subclass, int progif, int revision)
{
    size_t index = after;
    while (++index < device_count) {
        pci_device_summary_t const& dev = devices[index];
        
        if ((dev_class < 0 || dev.dev_class == dev_class) &&
                (subclass < 0 || dev.subclass == subclass) &&
                (progif < 0 || dev.prog_if == progif) &&
                (revision < 0 || dev.revision == revision))
            return index;
    }

    return size_t(-1);
}

uint64_t pci_bar_size(size_t index, size_t bar)
{
    if (index >= device_count)
        return false;
    
    pci_device_summary_t const &dev = devices[index];
    
    return dev.bar_sizes[bar];
}

void pci_when_pci_ready(pci_ready_node_t *node, 
        pci_ready_callback_t callback, void *callback_arg)
{
    if (pci_ready_already)
        return callback(callback_arg);
    
    // Link node into chain
    assert(node->magic == node->expected_magic);
    node->callback = callback;
    node->callback_arg = callback_arg;
    node->next = pci_ready_node_first;
    pci_ready_node_first = node;
}

void pci_notify_pci_ready()
{
    assert(!pci_ready_already);
    if (pci_ready_already)
        return;
    
    // Walk the chain, reversing the order of the links on the way
    pci_ready_node_t *prev = nullptr;
    pci_ready_node_t *curr = pci_ready_node_first;
    
    // Reverse the order of the chain
    while (curr) {
        assert(curr->magic == curr->expected_magic);
        pci_ready_node_t *save_next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = save_next;
    }
    curr = prev;
    
    // Execute the items in the order they were added
    while (curr) {
        assert(curr->magic == curr->expected_magic);
        curr->callback(curr->callback_arg);
        curr = curr->next;
    }
    
    pci_ready_already = true;
}
