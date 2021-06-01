
/* -2M */
. = 0xFFE00000;

MEMORY {
	/* 512MB at +2MB */
	ram          (w) : org =   0x200000, len = 0x10000000
	
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
	
	___data_vma = LOADADDR(.data);
	
	.entry ORIGIN(entry) : AT(ORIGIN(entry)) {
		*(.text.entry);
		. = 16;
	}

	.text.early ORIGIN(early) : AT(ORIGIN(early)) {
		*(.text.early);
	}
	
	. = ALIGN(64);
	
	.rodata.early ADDR(.text.early)+SIZEOF(.text.early) 
		: AT(LOADADDR(.text.early) + SIZEOF(.text.early)) {
		*(.rodata.early);		
	} =0

	. = ALIGN(64);
	
	.text ORIGIN(rom) : AT(ORIGIN(rom)) {
		*(.text);
		*(.text.*);
				
		. = ALIGN(64);
	}
	
	.rodata ADDR(.text) + SIZEOF(.text) : 
			AT(LOADADDR(.text) + SIZEOF(.text)) {
		*(.rodata);
		*(.rodata.*);
		
		. = ALIGN(64);
	} =0

	.data ORIGIN(ram) : 
			AT(LOADADDR(.rodata) + SIZEOF(.rodata)) {
		___data_vma = .;
		*(.data);
		*(.data.*);
		___data_end = .;
		. = ALIGN(64);
	} =0
	
	___data_lma = LOADADDR(.data);
	
	.bss ADDR(.data) + SIZEOF(.data) (NOLOAD) : {
		___bss_st = .;
		*(.bss);
		*(.bss.*);
		*(COMMON)
		*(COMMON.*)
		*(COMMON*)
		___bss_en = .;
	} >ram
}
