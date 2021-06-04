
/* -2M */
. = 0xFFE00000;

MEMORY {
    /* 512MB at +2MB */
    ram          (w) : org = 0x00200000, len = 0x10000000

    /* -2M */
    rom       (rxai) : org = 0xFFE00000, len = 0x200000

    /* -64K */
    early            : org = 0xFFFF0000, len = 0x10000

    /* -16 */
    entry            : org = 0xFFFFFFF0, len = 0x10
}

SECTIONS {
    /DISCARD/ : { *(.note.gnu.build-id); }
    /DISCARD/ : { *(.eh_frame); }

    /* 386 and newer fetch the first instruction from 0xFFFFFFF0 */
    .entry : {
        *(.text.entry);
        . = 16;
    } >entry AT >entry

    /* Initial cs.base=0xFFFF0000, so have a region for that top 64K */
    .text.early : {
        *(.text.early);
        . = ALIGN(4K);
    } >early AT >early

    .rodata.early : {
        *(.rodata.early);
        . = ALIGN(4K);
    } >early AT >early =0

    .rodata : {
        *(.rodata);
        *(.rodata.*);

        . = ALIGN(4K);
    } >rom AT >rom =0

    .text : {
        *(.text);
        *(.text.*);

        . = ALIGN(4K);
    } >rom AT >rom

    .data : {
        ___data_vma = .;
        *(.data);
        *(.data.*);
        ___data_end = .;
        . = ALIGN(16K);
    } >ram AT >rom =0
    
    . = ALIGN(16K);

    .bss (NOLOAD) : {
        ___bss_st = .;
        *(.bss);
        *(.bss.*);
        *(COMMON)
        *(COMMON.*)
        *(COMMON*)
        . = ALIGN(16K);
        *(.bss.align16k);
        *(.bss.align16k.*);
        *(.bss.align4k);
        *(.bss.align4k.*);
        *(.bss.align64);
        *(.bss.align64.*);
        *(.bss.align8);
        *(.bss.align8.*);
        ___bss_en = .;
        
        ___heap_st = ALIGN(4K);
    } >ram AT >ram
    
    ___initial_stack = ORIGIN(ram);

    /* Create a symbol we can use to locate the start of the .data RAM */
    ___data_vma = ADDR(.data);
    
    /* Create a symbol we can use to locate the start of the .data ROM image */
    ___data_lma = LOADADDR(.data);
}
