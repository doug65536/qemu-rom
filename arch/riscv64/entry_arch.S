.section .text.entry, "x", @progbits

entry:
    // Total cheat for now, real machines may not start hardid zero
    csrr a0, mhartid
    bne a0,zero,idle_trap

    // ROM is so far away from RAM, compiled code can't reach it
    // so just copy the ROM image to RAM, jump there, and abandon the ROM
    la a1,___image_end
    la a2,___image_vma
    la a3,___image_lma
.Lcopy_more_data:
    ld a4,(a3)
    sd a4,(a2)
    add a3,a3,8
    add a2,a2,8
    bgtu a1,a2,.Lcopy_more_data
    
    la a1,___bss_en
    la a2,___bss_st
.Lzero_more_bss:
    sd zero,(a2)
    add a2,a2,8
    bgtu a1,a2,.Lzero_more_bss
    
    call main

idle_trap:
    wfi
    j idle_trap

