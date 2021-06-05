#include "pci.h"
#include "debug.h"
#include "assert.h"

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

void set_bars(pci_addr const& addr, uint32_t &mm_next_place, uint32_t mm_limit,
        uint32_t &io_next_place, uint32_t *alignments)
{
    uint32_t bars[5];
    size_t base_addr_ofs = offsetof(pci_config_hdr_t, base_addr);

    for (size_t i = 0; i < 5; ++i) {
        uint32_t bar_ofs = base_addr_ofs + (sizeof(*bars) * i);
        
        printdbg("bar_ofs %zu\n", (size_t)bar_ofs);

        uint32_t bar = pci_read(addr, sizeof(uint32_t), bar_ofs);
        
        uint32_t next_bar = 0;
        
        bool is64 = (bar & 6) == 4;
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

        uint32_t base;
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
