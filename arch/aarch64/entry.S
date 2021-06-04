
.section .text.entry, "", @progbits

entry:
    mrs x0,CurrentEL
    lsr x0,x0,#2
    and x0,x0,#3
    cmp x0,3
    b.eq .Lis_el3
    cmp x0,2
    b.eq .Lis_el2
    cmp x0,1
    b.eq .Lis_el1

.Lis_el1:   // TODO
.Lis_el2:   // TODO
.Lis_el3:   // TODO
    
.Ldone_el_init:
    # Go sleep if not BSP
    mrs x0,MPIDR_EL1
    and x0,x0,#0xff
    cbnz x0,idle_trap
    
    ldr x0,=___initial_stack
    mov sp,x0
    bl main

idle_trap:
    wfi
    b idle_trap
