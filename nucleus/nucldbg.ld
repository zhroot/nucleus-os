/*
* Linker script used when building image for symbols/disassembly
*
* v0.1 ???
* v0.2 Doug Gale
*	- Linker now enforces header block is at beginning of image
*	- Added preliminary support for dynamic linking
*	- Added section alignment. Uses page alignment where appropriate
*	- Added support for __attribute__((constructor/destructor))
*	- Added debugging information sections for various formats
*	- Added symbols used in source to resolve various memory ranges
* v0.3 Doug Gale
*	- Fixed bug with COMMON input section
*/

SECTIONS
{
	. = 0x00100000;

	.text : {
		___text_st = .;

		*(.head);

		/* Pointers for the dynamic linker to find import & export tables */
		LONG(___export_st);
		LONG(___export_en);
		LONG(___import_st);
		LONG(___import_en);

		. = ALIGN(0x1000);		/* 4K align */
		*(.text);

		. = ALIGN(64);			/* Cache align */
		/* Export table */
		___export_st = .;
		*(.export);
		___export_en = .;
		*(.expnam);

		. = ALIGN(64);			/* Cache align */
		/* Import table */
		___import_st = .;
		*(.import);
		___import_en = .;
		*(.impnam);

		/* Constructors */
		___ctors_st = .;
		*SORT(.ctor);
		*SORT(.ctors);
		LONG(0);
		___ctors_en = .;

		/* Destructors */
		___dtors_en = .;
		LONG(0);
		*SORT(.dtors);
		*SORT(.dtor);
		___dtors_st = .;

		___text_en = .;
		. = ALIGN(0x1000);		/* 4K align */
	}

	.data : {
		___data_st = .;

		*(.data);

		. = ALIGN(0x1000);		/* 4K align */

		___data_en = .;
	}

	___load_end_addr = .;

	.bss : {
		___bss_st = .;

		*(.bss);
		*(COMMON);

		. = ALIGN(0x1000);		/* 4K align */

		___bss_en = .;

	}

	/* Stabs debugging sections.  */
	.stab			0 : { *(.stab) }
	.stabstr		0 : { *(.stabstr) }
	/* DWARF 2 */
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }
	.debug_info		0 : { *(.debug_info) *(.gnu.linkonce.wi.*) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line		0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str		0 : { *(.debug_str) }
	.debug_loc		0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
}

