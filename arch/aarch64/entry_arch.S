#include "cfi_helpers.h"

.section .text.entry, "x", @progbits

entry:
    .cfi_startproc simple
    .cfi_def_cfa CFI_SP,0
    .cfi_undefined CFI_PC    
    
    ldr x0,=___initial_stack
    mov sp,x0
    
    // Initialize vector table
    ldr x0,=vbar
    msr VBAR_EL3,x0
    
    mrs x0,CurrentEL
    lsr x0,x0,#2
    and x0,x0,#3
    cmp x0,3
    b.eq .Lis_el3
    cmp x0,2
    b.eq .Lis_el2
    cmp x0,1
    b.eq .Lis_el1
    
.Lis_el3:
.Lis_el1:   // TODO
.Lis_el2:   // TODO

.Ldone_el_init:
    // Go sleep if not BSP
    mrs x0,MPIDR_EL1
    and x0,x0,#0xff
    cbnz x0,idle_trap
    
    ldr x0,=___data_end
    ldr x1,=___data_vma
    ldr x2,=___data_lma
.Lcopy_more_data:
    ldp x3,x4,[x2],#16
    stp x3,x4,[x1],#16
    cmp x1,x0
    b.lt .Lcopy_more_data
    
    ldr x0,=___bss_en
    ldr x1,=___bss_st
.Lzero_more_bss:
    stp xzr,xzr,[x1],#16
    cmp x1,x0
    b.lt .Lzero_more_bss
    
    bl main

idle_trap:
    msr daifset,#0xF
    wfi
    b idle_trap

call_ctors:
    ldr x20,=__init_array_start
    ldr x21,=__init_array_end
.Lcall_another_ctor:
    ldr x0,[x20],#8
    cbz x0,.Ldone_ctor
    blr x0
    cmp x20,x21
    b.lt .Lcall_another_ctor
.Ldone_ctor:

    .cfi_endproc

.macro vbar_entry_stub vbar_ofs
.cfi_startproc simple
.cfi_signal_frame
.cfi_def_cfa sp,0
.cfi_return_column 65
0:
b .
. = 0b + 128
.cfi_endproc
.endm

.section .text.vbar, "x", @progbits
.global vbar
vbar:

// 16 entry points

vbar_0x000_synchronous:     vbar_entry_stub 0x000
vbar_0x080_irq:             vbar_entry_stub 0x080
vbar_0x100_fiq:             vbar_entry_stub 0x100
vbar_0x180_serror:          vbar_entry_stub 0x180
vbar_0x200_synchronous_spx: //vbar_entry_stub 0x200

.cfi_startproc simple
.cfi_signal_frame
.cfi_def_cfa sp,0
//.cfi_return_column 65
//.cfi_return_column CFI_ELR_mode
.cfi_register CFI_PC,CFI_ELR_mode
.cfi_same_value CFI_SP
.cfi_same_value CFI_X30
wtf:
0:
b crashed
. = 0b + 128
.cfi_endproc

vbar_0x280_irq:             vbar_entry_stub 0x280
vbar_0x300_fiq:             vbar_entry_stub 0x300
vbar_0x380_serror:          vbar_entry_stub 0x380
vbar_0x400_synchronous:     vbar_entry_stub 0x400
vbar_0x480_irq:             vbar_entry_stub 0x480
vbar_0x500_fiq:             vbar_entry_stub 0x500
vbar_0x580_serror:          vbar_entry_stub 0x580
vbar_0x600_synchronous_spx: vbar_entry_stub 0x600
vbar_0x680_irq:             vbar_entry_stub 0x680
vbar_0x700_fiq:             vbar_entry_stub 0x700
vbar_0x780_serror:          vbar_entry_stub 0x780

crashed:
.set i, 0
.rept 128
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_return_column (i)
    .cfi_def_cfa CFI_SP,0
    
    mov x0,x0
    
    .cfi_endproc
    .set i, i + 1
.endr

    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_return_column CFI_ELR_mode // 33
    .cfi_def_cfa CFI_SP,0   // 31
    .cfi_same_value CFI_X30 //30

0:  wfi
    wfe
    b 0b

    .cfi_endproc
