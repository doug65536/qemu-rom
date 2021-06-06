// ---------------------------------------------------------------------------
.code16

//
// CR4

#define CPU_CR4_PSE_BIT         4	// Page Size Extension
#define CPU_CR4_PAE_BIT         5	// Physical Address Extension
#define CPU_CR4_PGE_BIT         7	// Page Global Enabled
#define CPU_CR4_OFXSR_BIT       9	// Support FXSAVE and FXRSTOR
#define CPU_CR4_OSXMMEX_BIT     10	// Support Unmasked SIMD FP Exceptions
#define CPU_CR4_OSXSAVE_BIT     18	// XSAVE

#define CPU_CR4_PSE             (1U << CPU_CR4_PSE_BIT     )
#define CPU_CR4_PAE             (1U << CPU_CR4_PAE_BIT     )
#define CPU_CR4_PGE             (1U << CPU_CR4_PGE_BIT     )
#define CPU_CR4_OFXSR           (1U << CPU_CR4_OFXSR_BIT   )
#define CPU_CR4_OSXMMEX         (1U << CPU_CR4_OSXMMEX_BIT )
#define CPU_CR4_OSXSAVE         (1U << CPU_CR4_OSXSAVE_BIT )

// At -16
.section .text.entry, "ax", @progbits
entry:
    .cfi_startproc simple
    .cfi_def_cfa esp,0
    .cfi_undefined esp
    .cfi_undefined eip
    jmp _start
    .cfi_endproc

// At -64KB
.section .text.early, "ax", @progbits
_start:
    .cfi_startproc simple
    .cfi_def_cfa esp,0
    .cfi_undefined esp
    .cfi_undefined eip
    mov $ ___initial_stack,%esp
    
    // Hold self test result in ebp until later
    mov %eax,%ebp
    
    // Enable protected mode, enable cache (CR0.CD=0, CR0.NW=0, CR0.PE=1)
    mov %cr0,%eax
    xor $ (3U << 29) | 1,%eax
    mov %eax,%cr0
    
    addr32 lgdtl gdt+2
    ljmpl $ 8,$ .Lcode32_entry
    .cfi_endproc

// ---------------------------------------------------------------------------
.code32

#if defined(__x86_64__)

// Adjustment factor
#define UNRELOCATED(p) ((p) - 0x100000000)

#else

// Adjustment factor
#define UNRELOCATED(p) (p)

#endif

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
    mov $ UNRELOCATED(___bss_st),%edi
    mov $ UNRELOCATED(___bss_en),%ecx
    sub %edi,%ecx
    xor %eax,%eax
    cld
    rep stosb
    
    // Initialize .data
    mov $ ___data_lma,%esi
    mov $ UNRELOCATED(___data_vma),%edi
    mov $ UNRELOCATED(___data_end),%ecx
    sub %edi,%ecx
    rep movsb
    
    mov %ebp,UNRELOCATED(post_result)
    
    // Enable SSE (CR4_OFXSR) (Promise we will context switch sse state)
    mov %cr4,%eax
    orl $ (CPU_CR4_OFXSR | CPU_CR4_PAE | CPU_CR4_PGE | CPU_CR4_PSE),%eax
    mov %eax,%cr4
    
#ifdef __x86_64__

#define CPU_MSR_EFER            0xC0000080U

#define CPU_MSR_EFER_LME_BIT    8
#define CPU_MSR_EFER_NX_BIT     11

#define CPU_MSR_EFER_LME        (1U << CPU_MSR_EFER_LME_BIT)
#define CPU_MSR_EFER_NX         (1U << CPU_MSR_EFER_NX_BIT)

    call init_page_tables
    
    jmp enter_long_mode
.Ldone_enter_long_mode:
.code64
#endif
    
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

    // code32
    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x9b      // type
    .byte 0xcf      // limit flags
    .byte 0         // base hi

    // data
    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x93      // type
    .byte 0xcf      // limit flags
    .byte 0         // base hi

    // code64
    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x9b      // type
    .byte 0xef      // limit flags (+L bit)
    .byte 0         // base hi
gdt_end:

.section .bss.align4k, "w"
.balign 4096
page_tables:
    // Need 1 for PML4
    // Plus 1 PDPE
    .space 8<<10

.section .data, "aw", @progbits
#if defined(__x86_64__) || defined(__aarch64__)
bump_alloc: .quad ___heap_st
#elif defined(__i386__)
bump_alloc: .int ___heap_st
#else
#error Unhandled architecture
#endif

#define PTE_PRESENT_BIT     0
#define PTE_WRITABLE_BIT    1
#define PTE_ACCESSED_BIT    5
#define PTE_DIRTY_BIT       6
#define PTE_PAGESIZE_BIT    7
#define PTE_PRESENT         (1ULL << PTE_PRESENT_BIT)
#define PTE_WRITABLE        (1ULL << PTE_WRITABLE_BIT)
#define PTE_PAGESIZE        (1ULL << PTE_PAGESIZE_BIT)
#define PTE_ACCESSED        (1ULL << PTE_ACCESSED_BIT)
#define PTE_DIRTY           (1ULL << PTE_DIRTY_BIT)

#ifdef __x86_64__

.code32
.section .text, "ax", @progbits
init_page_tables:
    // Write present and writable entry pointing to PDPT
    mov $ UNRELOCATED(page_tables + 4096 + PTE_PRESENT + PTE_WRITABLE),%ecx
    mov %ecx,UNRELOCATED(page_tables)
    
    mov $ UNRELOCATED(page_tables + 4096),%edx
    mov $ 1 << 30,%eax
    
    mov $ PTE_PRESENT + PTE_WRITABLE + PTE_PAGESIZE,%ecx
    mov %ecx,(%edx)
    mov %ecx,4*8(%edx)
    
    add %eax,%ecx
    mov %ecx,1*8(%edx)
    
    add %eax,%ecx
    mov %ecx,2*8(%edx)

    add %eax,%ecx    
    mov %ecx,3*8(%edx)
    
    mov $ UNRELOCATED(page_tables),%eax
    mov %eax,%cr3

    ret
    
enter_long_mode:
    mov $ CPU_MSR_EFER,%ecx
    rdmsr
    or $ CPU_MSR_EFER_LME,%eax
    wrmsr
    
    // Enable paging
    mov %cr0,%eax
    or $ 1 << 31,%eax
    mov %eax,%cr0
    
    // Reload cs to pick up L bit
    ljmp $ 0x18,$ long_mode_entry

.code64
long_mode_entry:
    movabs $ 0x100000000,%rax
    // Use stack in intended data area
    lea (%rsp,%rax),%rsp
    jmp .Ldone_enter_long_mode

#endif  // def __x86_64__
