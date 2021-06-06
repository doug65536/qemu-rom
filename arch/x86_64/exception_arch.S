

#define INTR_EX_BASE        0

#define INTR_EX_DIV         0       // #DE
#define INTR_EX_DEBUG       1       // #DB
#define INTR_EX_NMI         2       // NMI
#define INTR_EX_BREAKPOINT  3       // #BP
#define INTR_EX_OVF         4       // #OF
#define INTR_EX_BOUND       5       // #BR
#define INTR_EX_OPCODE      6       // #UD
#define INTR_EX_DEV_NOT_AV  7       // #NM
#define INTR_EX_DBLFAULT    8       // #DF
#define INTR_EX_COPR_SEG    9       // obsolete, reserved
#define INTR_EX_TSS         10      // #TS
#define INTR_EX_SEGMENT     11      // #NP
#define INTR_EX_STACK       12      // #SS
#define INTR_EX_GPF         13      // #GP
#define INTR_EX_PAGE        14      // #PF
#define INTR_EX_MATH        16      // #MF
#define INTR_EX_ALIGNMENT   17      // #AC
#define INTR_EX_MACHINE     18      // #MC
#define INTR_EX_SIMD        19      // #XF
#define INTR_EX_VIRTUALIZE  20      // (intel)
#define INTR_EX_VMMCOMM     29      // #VC (AMD)
#define INTR_EX_SECURITY    30      // #SX (AMD)

// 21-31 are reserved for future exceptions
#define INTR_EX_LAST        31

// Vectors >= 32 go through generic intr_invoke codepath
#define INTR_SOFT_BASE      32

// Spurious handler must not EOI, so it is outside APIC dispatch range
// Needs to be on a multiple of 8 on some (old) processors
#define INTR_APIC_SPURIOUS  32

// Not really used, but yielded context is written using this interrupt number
#define INTR_THREAD_YIELD   33

// 34-39 reserved

#define INTR_SOFT_LAST      39

// Relatively hot area of the IDT, cache line aligned
// Everything following here needs LAPIC EOI
#define INTR_APIC_DSP_BASE  40
#define INTR_IPI_PANIC      40
#define INTR_APIC_ERROR     41
#define INTR_APIC_THERMAL   42

// Vectors >= 44 go through apic_dispatcher codepath
#define INTR_APIC_TIMER     43
#define INTR_IPI_TLB_SHTDN  44
#define INTR_IPI_RESCHED    45
#define INTR_IPI_FL_TRACE   46
// 47 reserved

// 192 vectors for IOAPIC and MSI(x)
#define INTR_APIC_IRQ_BASE  48
#define INTR_APIC_IRQ_END   239
#define INTR_APIC_DSP_LAST  239
#define INTR_APIC_IRQ_COUNT (INTR_APIC_DSP_LAST - INTR_APIC_IRQ_BASE + 1)
// Everything preceding here needs LAPIC EOI

// Everything following here needs PIC EOI
// Vectors >= 240 go through pic_dispatcher codepath
// PIC IRQs 0xF0-0xFF
#define INTR_PIC_DSP_BASE   240
#define INTR_PIC1_IRQ_BASE  240
#define INTR_PIC1_SPURIOUS  247
#define INTR_PIC2_IRQ_BASE  248
#define INTR_PIC2_SPURIOUS  255
#define INTR_PIC_DSP_LAST   255
// Everything preceding here needs PIC EOI

#define INTR_COUNT 256

.section .text

// Context frame
// CFA
// index
//          frame      raw
//   0    8*8(%rbp) 25*8(%rsp) <-- CFA
//  -1    7*8(%rbp) 24*8(%rsp) -> ss            ┐
//  -2    6*8(%rbp) 23*8(%rsp) -> rsp           │
//  -3    5*8(%rbp) 22*8(%rsp) -> flags         │
//  -4    4*8(%rbp) 21*8(%rsp) -> cs            ├ interrupt frame
//  -5    3*8(%rbp) 20*8(%rsp) -> rip           │
//  -6    2*8(%rbp) 19*8(%rsp) -> error code    │
//  -7    1*8(%rbp) 18*8(%rsp) -> interrupt     ┘
//  -8    0*8(%rbp) 17*8(%rsp) -> caller's rbp  ┐ <- &*rbp
//  -9   -1*8(%rbp) 16*8(%rsp) -> rbx           │
// -10   -2*8(%rbp) 15*8(%rsp) -> r15           ├ call preserved
// -11   -3*8(%rbp) 14*8(%rsp) -> r14           │
// -12   -4*8(%rbp) 13*8(%rsp) -> r13           │
// -13   -5*8(%rbp) 12*8(%rsp) -> r12           ┘
// -14   -6*8(%rbp) 11*8(%rsp) -> r11           ┐
// -15   -7*8(%rbp) 10*8(%rsp) -> r10           │
// -16   -8*8(%rbp)  9*8(%rsp) -> rax           │
// -17   -9*8(%rbp)  8*8(%rsp) -> r9            │
// -18  -10*8(%rbp)  7*8(%rsp) -> r8            ├ call clobbered
// -19  -11*8(%rbp)  6*8(%rsp) -> rcx           │
// -20  -12*8(%rbp)  5*8(%rsp) -> rdx           │
// -21  -13*8(%rbp)  4*8(%rsp) -> rsi           │
// -22  -14*8(%rbp)  3*8(%rsp) -> rdi           ┘
// -23  -15*8(%rbp)  2*8(%rsp) -> cr3
// -24  -16*8(%rbp)  1*8(%rsp) -> segments
// -25  -17*8(%rbp)  0*8(%rsp) -> fpu ctx ptr     <- Saved thread rsp

.macro isr_label i
.global isr_\()\i
isr_\i :
0:
.endm

.section .text.align4k, "x", @progbits
interrupt_entry_points:
.set i, 0
.rept 256
    .cfi_startproc simple
    .cfi_signal_frame
   
    //isr_label i
0:    
    // 8, 10, 11, 12, 13, 14, 17, 21
    .if i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21
        .cfi_def_cfa rsp,48
        .cfi_offset ss,-1*8
        .cfi_offset rsp,-2*8
        .cfi_offset rflags,-3*8
        .cfi_offset cs,-4*8
        .cfi_offset rip,-5*8
        // error code here
    .else
        .cfi_def_cfa rsp,40
        .cfi_offset ss,-1*8
        .cfi_offset rsp,-2*8
        .cfi_offset rflags,-3*8
        .cfi_offset cs,-4*8
        .cfi_offset rip,-5*8
        push $ 0
    .endif
    push $ i
    jmp isr_common
    
    . = 0b + 16;
    .cfi_endproc
    
    .set i, i + 1
.endr

.balign 16
.global isr_common
isr_common:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa rsp,40
    .cfi_offset ss,-1*8
    .cfi_offset rsp,-2*8
    .cfi_offset rflags,-3*8
    .cfi_offset cs,-4*8
    .cfi_offset rip,-5*8

    sub $ 25 * 8,%rsp
    .cfi_adjust_cfa_offset 25 * 8
    movq %rbp,17*8(%rsp)
    movq %rbx,16*8(%rsp)
    .cfi_offset rbp,-8*8
    .cfi_offset rbx,-9*8
    
    // Outgoing context pointer preserved in rbx
    mov %rsp,%rbx
    
    movq %r15,15*8(%rsp)
    movq %r14,14*8(%rsp)
    .cfi_offset r15,-10*8
    .cfi_offset r14,-11*8
    mov %ds,%r15d
    mov %es,%r14d
    
    movq %r13,13*8(%rsp)
    movq %r12,12*8(%rsp)
    .cfi_offset r13,-12*8
    .cfi_offset r12,-13*8
    mov %fs,%r13d
    mov %gs,%r12d
    
    movq %r11,11*8(%rsp)
    movq %r10,10*8(%rsp)
    .cfi_offset r11,-14*8
    .cfi_offset r10,-15*8
    
    shl $ 16,%r14d
    shl $ 32,%r13
    
    movq %rax, 9*8(%rsp)
    movq  %r9, 8*8(%rsp)
    .cfi_offset rax,-16*8
    .cfi_offset  r9,-17*8
    
    shl $ 48,%r12
    orq %r14,%r15
    
    movq  %r8, 7*8(%rsp)
    movq %rcx, 6*8(%rsp)
    .cfi_offset  r8,-18*8
    .cfi_offset rcx,-19*8
    
    orq %r13,%r12
    
    movq %rdx, 5*8(%rsp)
    movq %rsi, 4*8(%rsp)
    .cfi_offset rdx,-20*8
    .cfi_offset rsi,-21*8
    
    mov %cr3,%r14
    orq %r12,%r15
    
    movq %rdi, 3*8(%rsp)
    movq %r14, 2*8(%rsp)
    .cfi_offset rdi,-22*8
    .cfi_offset rcx,-19*8
    
    // Pass context pointer to 
    mov %rsp,%rdi
    
    movq %r15, 1*8(%rsp)
    movq $ 0,     (%rsp)
    
    test %r15,%r15
    jnz .Lbad_segments
.Lgood_segments:
    call isr_dispatcher
    mov %rax,%rsp
    
    
    
.Lbad_segments:
    xor %eax,%eax
    mov %eax,%ds
    mov %eax,%es
    mov %eax,%fs
    mov %eax,%gs
    jmp .Lgood_segments

.cfi_endproc

isr_dispatcher:
    mov %rdi,%rax
    ret
    
