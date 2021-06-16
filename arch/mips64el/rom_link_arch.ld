/* 000000001e000000-000000001e3fffff (prio 0, romd): mips_malta.bios */
/* 0000000080000000-00000000dfffffff (prio 0, ram): mips_malta.ram */

MEMORY {
    header        : org = 0x1e000000, len =     0x40    
    rom  (r)      : org = 0x1e000040, len = 0x1fefc0    
    entry         : org = 0x3ffff000, len =   0x1000
                  
    romram (w)    : org = 0x80000000, len =   0x400000
    ram           : org = 0x80400000, len = 0x5fcfffff
}

SECTIONS {
    /DISCARD/ : {
         *(.note.gnu.build-id);
         *(.eh_frame);
         *(.gnu.attributes);
    }
    
    /* The largest memory model can't reach RAM from ROM PC, 
     * so copy everything to RAM */
    
    .header : {
        ___image_lma = LOADADDR(.text);
        *(.header)
        *(.header.*)
    } >header AT >header

    .entry : {
        ___entry_vma = ABSOLUTE(.);
        *(.text.entry);
        *(.text.entry.*);
        
        . = ABSOLUTE(___entry_vma + 0x1000);
        ___image_end = ABSOLUTE(.);        
    } >entry AT >entry
    
    .text : {
        ___image_vma = ABSOLUTE(.);
        *(.text);
        *(.text.*);

        . = ALIGN(16K);
    } >romram AT >rom

    .rodata : {
        ___image_end_v = .; QUAD(___image_end);
        ___image_vma_v = .; QUAD(___image_vma);
        ___image_lma_v = .; QUAD(___image_lma);
        ___image_adj_v = .; QUAD(___image_lma - ___image_vma);
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
    } >romram AT >rom =0

/*    .debug : {    
        *(.debug_abbrev);
        *(.debug_aranges);
        *(.debug_frame);
        *(.debug_info);
        *(.debug_line);
        *(.debug_ranges);
        *(.debug_str);
    } >romram AT >rom = 0
*/
    .data : {
        *(.data);
        *(.data.*);
        . = ALIGN(16K);
        ___data_end = ABSOLUTE(.);
    } >romram AT >rom =0
    
    .sdata : {
        *(.sdata);
        *(SORT(.sdata.*));
    } >romram AT >rom =0
    
    .sbss (NOLOAD) : {
        ___bss_st = .;
        *(.sbss);
        *(SORT(.sbss.*));
    } >romram
    
    .bss (NOLOAD) : {
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
    } >romram
   
    ___initial_stack = ADDR(.bss) + SIZEOF(.bss) + 0x200000;
}
