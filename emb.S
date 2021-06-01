// ---------------------------------------------------------------------------
.code16

// At -64KB
.section .text.early, "ax", @progbits
_start:
    .cfi_startproc simple
    .cfi_def_cfa esp,0
    .cfi_undefined esp
    .cfi_undefined eip
    mov $ 0x90000,%esp
    
    // Hold self test result in ebp until later
    mov %eax,%ebp
    
    // Enable protected mode, enable cache (CR0.CD=0, CR0.NW=0, CR0.PE=1)
    mov %cr0,%eax
    xor $ (3U << 29) | 1,%eax
    mov %eax,%cr0
    
    addr32 lgdtl gdt+2
    ljmpl $ 8,$ .Lcode32_entry
    .cfi_endproc

// At -16
.section .text.entry, "ax", @progbits
entry:
    .cfi_startproc simple
    .cfi_def_cfa esp,0
    .cfi_undefined esp
    .cfi_undefined eip
    jmp _start
    .cfi_endproc

// ---------------------------------------------------------------------------
.code32

.section .text.early, "ax", @progbits
.Lcode32_entry:
    .cfi_startproc simple
    .cfi_def_cfa esp,0
    .cfi_undefined esp
    .cfi_undefined eip
    movl $ 0x10,%eax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%fs
    movw %ax,%gs
    movw %ax,%ss
    
    xor %ecx,%ecx
    lldt %cx
    
    // Clear bss
    mov $ ___bss_st,%edi
    mov $ ___bss_en,%ecx
    sub %edi,%ecx
    xor %eax,%eax
    cld
    rep stosb
    
    // Initialize .data
    mov $ ___data_lma,%esi
    mov $ ___data_vma,%edi
    mov $ ___data_end,%ecx
    sub %edi,%ecx
    rep movsb
    
    mov %ebp,post_result
    
    call main
    
0:  hlt
    jmp 0b
    .cfi_endproc

.section .data, "aw", @progbits
post_result:
    .int 0

check_data:
    .int 42

.section .rodata.early, "", @progbits
.balign 4
gdt:
    .short 0
    .short gdt_end - gdt - 1
    .int gdt

    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x9b      // type
    .byte 0xcf      // limit flags
    .byte 0         // base hi

    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x93      // type
    .byte 0xcf      // limit flags
    .byte 0         // base hi
gdt_end:
