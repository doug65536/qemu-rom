
. = 0;

MEMORY {
    /* 512MB at +2MB */

    /* 0000000000000000-0000000003ffffff (prio 0, romd): virt.flash0 */
    rom      : org = 0x00000000, len = 0x4000000
    
    /* 0000000004000000-0000000007ffffff (prio 0, romd): virt.flash1 */
    nvram    : org = 0x04000000, len = 0x4000000

    /* 0000000040000000-0000000047ffffff (prio 0, ram): mach-virt.ram */
    ram  (w) : org = 0x40000000, len = 0x8000000
}

SECTIONS {
    /DISCARD/ : { *(.note.gnu.build-id); }
    /DISCARD/ : { *(.eh_frame); }

    /* First instruction fetched from ip=0 */
    .text : {
        *(.text.entry);
        
        . = ALIGN(2K);
        *(.text.vbar);
        
        *(.text);
        *(.text.*);

        . = ALIGN(4K);
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
        . = ALIGN(4K);
    } >rom AT >rom =0

/*    .debug : {    
        *(.debug_abbrev);
        *(.debug_aranges);
        *(.debug_frame);
        *(.debug_info);
        *(.debug_line);
        *(.debug_ranges);
        *(.debug_str);
    } >rom AT >rom =0
*/
    .data : {
        ___data_vma = .;
        *(.data);
        *(.data.*);
        ___data_end = .;
        . = ALIGN(16);
    } >ram AT >rom =0

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
