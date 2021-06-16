
. = 0;

MEMORY {
    /* 512MB at +2MB */
    
    
    ram  (w) : org = 0x0, len = 0x3fe00000
    
    header   : org = 0x3fe00000, len = 0x40
    
    rom      : org = 0x3fe00040, len = 0x1fefc0
    
    entry    : org = 0x3ffff000, len = 0x1000
}

SECTIONS {
    /DISCARD/ : { *(.note.gnu.build-id); }
    /DISCARD/ : { *(.eh_frame); }
    
    /* The largest memory model can't reach RAM from ROM PC, 
     * so copy everything to RAM on riscv64 */
    
    .header : {
        *(.header)
        *(.header.*)
    } >header AT >header

    /* First instruction fetched from ip=0xFFF00100 */
    .entry : {
        ___image_vma = ABSOLUTE(.);
        ___image_lma = LOADADDR(.text);
        *(.text.entry);
        *(.text.entry.*);
        
        . = ABSOLUTE(0x40000000);
    } >entry AT >entry
    
    .text : {
        *(.text);
        *(.text.*);

        . = ALIGN(16K);
    } >rom AT >rom

    .rodata : {
        *(.rodata);
        *(.rodata.*);
        
		PROVIDE(__init_array_start = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
		PROVIDE(__init_array_end = .);

		PROVIDE(__init_array_early_start = .);
		KEEP(*(.ctors))
		KEEP(*(.init_array))
		PROVIDE(__init_array_early_end = .);

		___dtors_st = .;
		KEEP(*(.dtors))
		KEEP(*(.dtors.*))
		KEEP(*(.fini_array))
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
		___dtors_en = .;
        
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
    } >rom AT >rom =0

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
    } >ram
   
    ___initial_stack = ADDR(.bss) + SIZEOF(.bss) + 0x200000;

    /* Create a symbol we can use to locate the start of the .data RAM */
    ___data_vma = ADDR(.data);
    
    /* Create a symbol we can use to locate the start of the .data ROM image */
    ___data_lma = LOADADDR(.data);
}
