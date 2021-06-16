#include "config.h"
#include "uboot.h"

.section .text.entry, "x", @progbits

// Register usage
//  0
//  1 stack ptr
//  2 toc ptr
//  3 1st argument
//  4
//  5
//  6
//  7
//  8
//  9
// 10 8th arg (last)
// 11 environment pointer
// 12 reserved for exception handling
// 13 reserved
// 14-31 callee saved

.global entry
entry:
    
    
.L_const_image_end: .long ___image_end
.L_const_image_vma: .long ___image_vma
.L_const_image_lma: .long ___image_lma


.section .header, "ax", @progbits
uboot_header:
be_int32 IH_MAGIC
be_int32 0  // header crc (qemu ignores it)
be_int32 0  // time
be_int32 0x1fffc0   // size
be_int32 0x3fe00040 // base
be_int32 0x3ffff000 // entry
be_int32 0  // data crc (qemu ignores it)
.byte IH_OS_U_BOOT
.byte IH_CPU_PPC
.byte IH_TYPE_KERNEL
.byte IH_COMP_NONE
name: .asciz "qemu-rom"
.space (name + 32) - .,0

//typedef struct uboot_image_header {
//uint32_t ih_magic; 
//uint32_t ih_hcrc; 
//uint32_t ih_time; 
//uint32_t ih_size; 
//uint32_t ih_load; 
//uint32_t ih_ep; 
//uint32_t ih_dcrc; 
//uint8_t ih_os; 
//uint8_t ih_arch; 
//uint8_t ih_type; 
//uint8_t ih_comp; 
//uint8_t ih_name[IH_NMLEN]; 
//} uboot_image_header_t;
