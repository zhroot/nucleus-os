SECTIONS
{
	. = 0x00000500;

	.text : {
		*(.head)

		. = ALIGN(64);		/* cache line align */
		*(.text)

		. = ALIGN(64);		/* cache line align */

		*(.data)
		*(.rodata)

		. = ALIGN(64);		/* cache line align */

		___bss_st = .;
		*(.bss)
		*(COMMON)
	}

	.data : {
		___allocator_start = .;

		. = ALIGN(0xfc00);		/* 1K buffer at top of memory */
		___stack_bottom = .;
		___load_buffer_1KB = .;

		___bss_en = .;
	}
}

