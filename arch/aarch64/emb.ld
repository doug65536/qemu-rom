
. = 0;

MEMORY {
    /* 512MB at +2MB */

    /* 0 */
    rom            : org = 0, len = 0x40000000
    
    nvram          : org = 0x40000000, len = 0x40000000

    /* 128M */
    ram          (w) : org = 0x40000000, len = 0x7ffffff
}

SECTIONS {
    /DISCARD/ : { *(.note.gnu.build-id); }
    /DISCARD/ : { *(.eh_frame); }

    /* First instruction fetched from ip=0 */
    .text : {
        *(.text.entry);
        *(.text);
        *(.text.*);

        . = ALIGN(16K);
    } >rom AT >rom

    .rodata : {
        *(.rodata);
        *(.rodata.*);

        . = ALIGN(16K);
    } >rom AT >rom =0

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
    
    ___initial_stack = ADDR(.bss) + SIZEOF(.bss) + 0x200000;

    /* Create a symbol we can use to locate the start of the .data RAM */
    ___data_vma = ADDR(.data);
    
    /* Create a symbol we can use to locate the start of the .data ROM image */
    ___data_lma = LOADADDR(.data);
}
