
.section .text.entry, "", @progbits

entry:
    mrs x0,CurrentEL
    ldr x0,=___initial_stack
    mov sp,x0
    b main
