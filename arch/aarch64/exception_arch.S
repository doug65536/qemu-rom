#include "cfi_helpers.h"

.global exception_handler
exception_handler:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa CFI_SP,0
    .cfi_return_column CFI_PC
    
    sub sp,sp,34*8
    .cfi_adjust_cfa_offset 34*8
    
    stp  x0, x1,[sp]
    mrs  x0, sp_el0
    stp  x2, x3,[sp, 1*16]
    mrs  x1, elr_el3
    stp  x4, x5,[sp, 2*16]
    mrs  x2, spsr_el3
    stp  x6, x7,[sp, 3*16]
    stp  x8, x9,[sp, 4*16]
    stp x10,x11,[sp, 5*16]
    stp x12,x13,[sp, 6*16]
    stp x14,x15,[sp, 7*16]
    stp x16,x17,[sp, 8*16]
    stp x18,x19,[sp, 9*16]
    stp x20,x21,[sp, 10*16]
    stp x22,x23,[sp, 11*16]
    stp x24,x25,[sp, 12*16]
    stp x26,x27,[sp, 13*16]
    stp x28,x29,[sp, 14*16]
    stp  x2,x30,[sp, 15*16]
    stp  x0, x1,[sp, 16*16]
    
    mov x0,sp
    //bl schedule
    
    ldp  x2,x30,[sp, 15*16]
    ldp  x0, x1,[sp, 16*16]
    msr  sp_el0,x0
    msr  elr_el3,x1
    msr  spsr_el3,x2
    
    ldp  x0, x1,[sp]
    ldp  x2, x3,[sp, 1*16]
    ldp  x4, x5,[sp, 2*16]
    ldp  x6, x7,[sp, 3*16]
    ldp  x8, x9,[sp, 4*16]
    ldp x10,x11,[sp, 5*16]
    ldp x12,x13,[sp, 6*16]
    ldp x14,x15,[sp, 7*16]
    ldp x16,x17,[sp, 8*16]
    ldp x18,x19,[sp, 9*16]
    ldp x20,x21,[sp, 10*16]
    ldp x22,x23,[sp, 11*16]
    ldp x24,x25,[sp, 12*16]
    ldp x26,x27,[sp, 13*16]
    ldp x28,x29,[sp, 14*16]
    
    .cfi_endproc
