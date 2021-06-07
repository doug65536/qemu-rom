
. = 0;

MEMORY {
    /* 512MB at +2MB */
    
    /* 1000-ffff (prio 0, rom): riscv.spike.mrom */
    rom      : org = 0x1000, len = 0xf000
    
    /* 80000000-87ffffff */
    /* but don't even touch 1st MB */
    ram  (w) : org = 0x80100000, len = 0x7F00000
}

SECTIONS {
    /DISCARD/ : { *(.note.gnu.build-id); }
    /DISCARD/ : { *(.eh_frame); }
    
    /* The largest memory model can't reach RAM from ROM PC, 
     * so copy everything to RAM on riscv64 */

    /* First instruction fetched from ip=0x1000 */
    .text : {
        ___image_vma = ABSOLUTE(.);
        ___image_lma = LOADADDR(.text);
        *(.text.entry);
        
        . = ALIGN(2K);
        *(.text.vbar);
        
        *(.text);
        *(.text.*);

        . = ALIGN(16K);
    } >ram AT >rom

    .rodata : {
        *(.rodata);
        *(.rodata.*);
        *(.got);
        *(.got.plt);
        *(.igot.plt);
        *(.iplt);
        *(.rela.data);
        *(.rela.got);
        *(.rela.iplt);
        *(.rela.text.early);
        *(.rela.text.entry);
        *(.comment);
        . = ALIGN(16K);
    } >ram AT >rom =0

/*    .debug : {    
        *(.debug_abbrev);
        *(.debug_aranges);
        *(.debug_frame);
        *(.debug_info);
        *(.debug_line);
        *(.debug_ranges);
        *(.debug_str);
    } >ram AT >rom = 0
*/
    .data : {
        *(.data);
        *(.data.*);
        . = ALIGN(16K);
        ___image_end = ABSOLUTE(.);        
    } >ram AT >ram =0
    
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
