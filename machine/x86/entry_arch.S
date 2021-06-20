// ---------------------------------------------------------------------------
.code16

//
// CR0

#define CPU_CR0_PE_BIT 0	// Protected Mode
#define CPU_CR0_MP_BIT 1	// Monitor co-processor
#define CPU_CR0_EM_BIT 2	// Emulation
#define CPU_CR0_TS_BIT 3	// Task switched
#define CPU_CR0_ET_BIT 4	// Extension type (0=80287)
#define CPU_CR0_NE_BIT 5	// Numeric error (1=exception, 0=IRQ)
#define CPU_CR0_WP_BIT 16	// Write protect (1=enforce in ring0)
#define CPU_CR0_AM_BIT 18	// Alignment mask (1=enable EFLAGS.AC)
#define CPU_CR0_NW_BIT 29	// Not-write through
#define CPU_CR0_CD_BIT 30	// Cache disable
#define CPU_CR0_PG_BIT 31	// Paging (1=enable paging)

#define CPU_CR0_PE  (1U << CPU_CR0_PE_BIT)
#define CPU_CR0_MP  (1U << CPU_CR0_MP_BIT)
#define CPU_CR0_EM  (1U << CPU_CR0_EM_BIT)
#define CPU_CR0_TS  (1U << CPU_CR0_TS_BIT)
#define CPU_CR0_ET  (1U << CPU_CR0_ET_BIT)
#define CPU_CR0_NE  (1U << CPU_CR0_NE_BIT)
#define CPU_CR0_WP  (1U << CPU_CR0_WP_BIT)
#define CPU_CR0_AM  (1U << CPU_CR0_AM_BIT)
#define CPU_CR0_NW  (1U << CPU_CR0_NW_BIT)
#define CPU_CR0_CD  (1U << CPU_CR0_CD_BIT)
#define CPU_CR0_PG  (1U << CPU_CR0_PG_BIT)

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
    //mov $ _start-0xFFFF0000,%bx
    mov $ _start-0xFFFF0000,%ebx
    jmp *%bx
    .cfi_endproc

// At -64KB
.section .text.early, "ax", @progbits
_start:
    .cfi_startproc simple
    .cfi_def_cfa esp,0
    .cfi_undefined esp
    .cfi_undefined eip

    // Hold self test result in ebp until later
    mov %eax,%ebp

//    mov $ 0x3f8,%edx
//    mov $ '!',%al
//    out %al,%dx

    // Enable protected mode, enable cache (CR0.CD=0, CR0.NW=0, CR0.PE=1)
    //mov %cr0,%eax
    mov $ CPU_CR0_CD | CPU_CR0_NW | CPU_CR0_ET | CPU_CR0_PE,%eax
    mov %eax,%cr0

    addr32 lgdtl %cs:gdt+2-0xFFFF0000
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
    mov $ UNRELOCATED(___stack_bottom),%esp

    // The upper 10 bits of eflags are undefined at reset. Clean them

    xor %ecx,%ecx
    push %ecx
    lldt %cx
    popfl

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

#if defined(__x86_64__)
#define PTR_SZ_ALIGN 8
#define PTR_SZ_VALUE .quad
#elif defined(__i386__)
#define PTR_SZ_ALIGN 4
#define PTR_SZ_VALUE .int
#else
#error Unhandled architecture
#endif

.hidden bump_alloc
.global bump_alloc
.balign PTR_SZ_ALIGN
bump_alloc: PTR_SZ_VALUE UNRELOCATED(___heap_st)

.section .bss
.balign PTR_SZ_ALIGN
page_tables: .space PTR_SZ_ALIGN,0

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
    .byte 0x9b      // type         | P |  DPL  |  1  | 1 | C | R | A |
    .byte 0xcf      // limit flags  | G | D | 0 | AVL |  LIMIT_19_16  |
    .byte 0         // base hi

    // data
    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x93      // type         | P |  DPL  |  1  | 0 | E | W | A |
    .byte 0xcf      // limit flags  | G | B | 0 | AVL |  LIMIT_19_16  |
    .byte 0         // base hi

    // code64
    .short 0xffff   // limit low
    .short 0        // base low
    .byte 0         // base mid
    .byte 0x9b      // type         | P |  DPL  |  1  | 1 | C | R | A |
    .byte 0xaf      // limit flags  | G | D | L | AVL |  LIMIT_19_16  |
    .byte 0         // base hi
gdt_end:

//.section .bss.align4k, "w"
//.balign 4096
//page_tables:
//    // Need 1 for PML4
//    // Plus 1 PDPE
//    .space 8<<10

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

.section .rodata.early, "", @progbits
.balign 16
pd_fill_pair:
    .quad PTE_PRESENT + PTE_WRITABLE + PTE_PAGESIZE
    .quad PTE_PRESENT + PTE_WRITABLE + PTE_PAGESIZE + (1<<21)
pd_fill_addend_2:
    .quad (2<<21)
    .quad (2<<21)
pd_fill_addend_8:
    .quad (8<<21)
    .quad (8<<21)

.code32
.section .text.early, "ax", @progbits
init_page_tables:
    push %edi

    // Use 2MB pages, create 1 PML4, 16 PDPT, 8192 PD
    //                       2^0      2^4      2^13
    // This early, we can assume bump_alloc is pointing far below 4GB
    mov $ UNRELOCATED(bump_alloc),%eax
    movl (%eax),%edi
    addl $ (8192 + 16 + 1) * 4096,(%eax)
    mov %edi,UNRELOCATED(page_tables)

    // Layout in memory
    // <-- %edi
    // 16MB of PD   <-- lowest address
    // 16 PDPE
    // PML4         <-- highest address
    // <-- bump_alloc

    // Fill in 2097152-2048, starting at entry 2048 PD entries
    xor %ecx,%ecx
    movdqa pd_fill_pair,%xmm0
    movdqa pd_fill_addend_2,%xmm4
    movdqa pd_fill_addend_8,%xmm5
    movdqa %xmm0,%xmm1
    paddq %xmm4,%xmm1
    movdqa %xmm1,%xmm2
    paddq %xmm4,%xmm2
    movdqa %xmm2,%xmm3
    paddq %xmm4,%xmm3
.Lanother_pd:
    movdqa %xmm0,2048*8(%edi,%ecx,8)
    paddq %xmm5,%xmm0
    movdqa %xmm1,16*1+ 2048*8(%edi,%ecx,8)
    paddq %xmm5,%xmm1
    movdqa %xmm2,16*2+ 2048*8(%edi,%ecx,8)
    paddq %xmm5,%xmm2
    movdqa %xmm3,16*3+ 2048*8(%edi,%ecx,8)
    paddq %xmm5,%xmm3
    add $ 8,%ecx
    cmp $ 2097152-2048,%ecx
    jne .Lanother_pd

    // 1st 2048 PD entries (1st 4GB)
    mfence
    xor %ecx,%ecx
.Lanother_low_pd:
    movdqa 16*0+ 2048*8(%edi,%ecx,8),%xmm0
    movdqa %xmm0,(%edi,%ecx,8)
    movdqa 16*1+ 2048*8(%edi,%ecx,8),%xmm0
    movdqa %xmm0,16*1(%edi,%ecx,8)
    movdqa 16*2+ 2048*8(%edi,%ecx,8),%xmm0
    movdqa %xmm0,16*2(%edi,%ecx,8)
    movdqa 16*3+ 2048*8(%edi,%ecx,8),%xmm0
    movdqa %xmm0,16*3(%edi,%ecx,8)
    add $ 8,%ecx
    cmp $ 2048,%ecx
    jne .Lanother_low_pd
    mov $ 2097152-2048,%ecx

    // Point edi at start of PDPT region
    xor %edx,%edx
    mov %edi,%eax
    lea (%edi,%ecx,8),%edi
    xor %ecx,%ecx
    or $ PTE_PRESENT + PTE_WRITABLE,%eax
.Lanother_pdpt:
    mov %eax,(%edi,%ecx,8)
    mov %edx,4(%edi,%ecx,8)
    add $ 4096,%eax
    adc $ 0,%edx
    add $ 1,%ecx
    cmp $ 16,%ecx
    jne .Lanother_pdpt

    pxor %xmm0,%xmm0
.Lzero_another_unused_pdpt:
    movdqa %xmm0,(%edi,%ecx,8)
    movdqa %xmm0,16*1(%edi,%ecx,8)
    movdqa %xmm0,16*2(%edi,%ecx,8)
    movdqa %xmm0,16*3(%edi,%ecx,8)
    add $ 8,%ecx
    cmp $ 512,%ecx
    jne .Lzero_another_unused_pdpt

    mov %edi,%eax
    lea (%edi,%ecx,8),%edi
    xor %ecx,%ecx
    or $ PTE_PRESENT + PTE_WRITABLE,%eax
    movd %eax,%xmm0
    pxor %xmm1,%xmm1
.Lwrite_another_pml4:
    movdqa %xmm0,(%edi,%ecx,8)
    movdqa %xmm1,16*1(%edi,%ecx,8)
    movdqa %xmm1,16*2(%edi,%ecx,8)
    movdqa %xmm1,16*3(%edi,%ecx,8)
    pxor %xmm0,%xmm0
    add $ 8,%ecx
    cmp $ 512,%ecx
    jne .Lwrite_another_pml4

    mov %edi,%cr3

    pop %edi
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
