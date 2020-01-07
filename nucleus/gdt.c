// gdt.c

// GDT definition and manipulation functions

// v0.1 Doug Gale
//	- Initial revision
//	- Added support for TSS
//	- Removed unnecessary descriptors with 64KB limit

#include <gdt.h>

typedef union {
	struct {
		unsigned desc_limit_0_15:16;	// Limit
		unsigned desc_base_0_15:16;		// Base
		unsigned desc_base_16_23:8;		// Base

		unsigned desc_accessed:1;		// Segment has been accessed
		unsigned desc_readable:1;		// 0=read-only, 1=read/write
		unsigned desc_conforming:1;		// 1=allow execution when RPL>CPL (?)
		unsigned desc_code:1;			// 0=data, 1=code
		unsigned desc_notspecial:1;		// 1=code/data descrip, 0=sys descrip

		unsigned desc_dpl:2;			// Descriptor priviledge level
		unsigned desc_present:1;		// Present
		unsigned desc_limit_16_19:4;	// Limit

		unsigned desc_osuse:1;			// Available for OS use
		unsigned :1;					// Reserved
		unsigned desc_use32:1;			// 0=16-bit default, 1=32-bit default
		unsigned desc_granularity:1;	// Limit granularity (0=bytes, 1=L*4096)

		unsigned desc_base_24_31:8;		// Base
	} bits;

	unsigned long long qw;				// Access as a quadword
} Descriptor_Code;

typedef union {
	struct {
		unsigned desc_limit_0_15:16;	// Limit
		unsigned desc_base_0_15:16;		// Base
		unsigned desc_base_16_23:8;		// Base

		unsigned desc_accessed:1;		// Segment has been accessed
		unsigned desc_writable:1;		// 0=read-only, 1=read/write
		unsigned desc_expanddown:1;		// 0=expandup, 1=expanddown
		unsigned :1;					// 0=data, 1=code
		unsigned desc_notspecial:1;		// 1=code/data descrip, 0=sys descrip

		unsigned desc_dpl:2;			// Descriptor priviledge level
		unsigned desc_present:1;		// Present
		unsigned desc_limit_16_19:4;	// Limit

		unsigned desc_osuse:1;			// Available for OS use
		unsigned :1;					// Reserved
		unsigned desc_use32:1;			// 0=16-bit default, 1=32-bit default
		unsigned desc_granularity:1;	// Limit granularity (0=bytes, 1=L*4096)

		unsigned desc_base_24_31:8;		// Base
	} bits;

	unsigned long long qw;				// Access as a quadword
} Descriptor_Data;

typedef union {
	struct {
		unsigned desc_limit_0_15:16;	// Limit

		unsigned desc_base_0_15:16;		// Base
		unsigned desc_base_16_23:8;		// Base

		unsigned desc_type:5;			// Segment type

		unsigned desc_dpl:2;			// Descriptor priviledge level
		unsigned desc_present:1;		// Present
		unsigned desc_limit_16_19:4;	// Limit
		unsigned desc_osuse:1;			// Available for OS use
		unsigned :1;					// Reserved
		unsigned desc_use32:1;			// 0=16-bit default, 1=32-bit default
		unsigned desc_granularity:1;	// Limit granularity (0=bytes, 1=L*4096)
		unsigned desc_base_24_31:8;		// Base
	} bits;

	unsigned long long qw;				// Access as a quadword
} Descriptor_Syst;

typedef union {
	Descriptor_Code sel_cod;		// Code selector
	Descriptor_Data sel_dat;		// Data selector
	Descriptor_Syst sel_sys;			// System selector
} Descriptor;

// System segment types (sel_type)
#define SELTYPE_TSS16		0x01
#define SELTYPE_TSS16BUSY	0x03
#define SELTYPE_CALLGATE16	0x04
#define SELTYPE_INTGATE16	0x06
#define SELTYPE_TRAPGATE16	0x07

#define SELTYPE_LDT			0x02
#define SELTYPE_TASKGATE	0x05
#define SELTYPE_TSS32		0x09
#define SELTYPE_TSS32BUSY	0x0B
#define SELTYPE_CALLGATE32	0x0C
#define SELTYPE_INTGATE32	0x0E
#define SELTYPE_TRAPGATE32	0x0F

// GDT
static Descriptor gdt[8];

// System GDT:
// |-----| --Selector Details--
// |     | 0x30 IDX=7 RPL=0 GDT Unused
// |     | 0x30 IDX=6 RPL=0 GDT Unused
// | TSS | 0x28 IDX=5 RPL=0 GDT
// | DS3 | 0x23 IDX=4 RPL=3 GDT
// | CS3 | 0x1B IDX=3 RPL=3 GDT
// | DS0 | 0x10 IDX=2 RPL=0 GDT
// | CS0 | 0x08 IDX=1 RPL=0 GDT
// | nul | 0x00
// |-----|

void gdt_init()
{
	// Code selector
	gdt[1].sel_cod.bits.desc_accessed = 1;
	gdt[1].sel_cod.bits.desc_code = 1;
	gdt[1].sel_cod.bits.desc_notspecial = 1;
	gdt[1].sel_cod.bits.desc_present = 1;
	gdt[1].sel_cod.bits.desc_use32 = 1;
	gdt[1].sel_cod.bits.desc_granularity = 1;		// 4GB limit
	gdt[1].sel_cod.bits.desc_limit_0_15 = 0xFFFF;	// 4GB limit
	gdt[1].sel_cod.bits.desc_limit_16_19 = 0xF;		// 4GB limit

	// Data selector
	gdt[2].sel_dat.bits.desc_accessed = 1;
	gdt[2].sel_dat.bits.desc_notspecial = 1;
	gdt[2].sel_dat.bits.desc_writable = 1;
	gdt[2].sel_dat.bits.desc_present = 1;
	gdt[2].sel_dat.bits.desc_use32 = 1;
	gdt[2].sel_dat.bits.desc_granularity = 1;		// 4GB limit
	gdt[2].sel_dat.bits.desc_limit_0_15 = 0xFFFF;	// 4GB limit
	gdt[2].sel_dat.bits.desc_limit_16_19 = 0xF;		// 4GB limit

	// Code selector (PL3)
	gdt[3].sel_cod.bits.desc_accessed = 1;
	gdt[3].sel_cod.bits.desc_notspecial = 1;
	gdt[3].sel_cod.bits.desc_code = 1;
	gdt[3].sel_cod.bits.desc_dpl = 3;
	gdt[3].sel_cod.bits.desc_present = 1;
	gdt[3].sel_cod.bits.desc_use32 = 1;
	gdt[3].sel_cod.bits.desc_granularity = 1;		// 4GB limit
	gdt[3].sel_cod.bits.desc_limit_0_15 = 0xFFFF;	// 4GB limit
	gdt[3].sel_cod.bits.desc_limit_16_19 = 0xF;		// 4GB limit

	// Data selector (PL3)
	gdt[4].sel_dat.bits.desc_accessed = 1;
	gdt[4].sel_dat.bits.desc_notspecial = 1;
	gdt[4].sel_dat.bits.desc_dpl = 3;
	gdt[4].sel_dat.bits.desc_writable = 1;
	gdt[4].sel_dat.bits.desc_present = 1;
	gdt[4].sel_dat.bits.desc_use32 = 1;
	gdt[4].sel_dat.bits.desc_granularity = 1;		// 4GB limit
	gdt[4].sel_dat.bits.desc_limit_0_15 = 0xFFFF;	// 4GB limit
	gdt[4].sel_dat.bits.desc_limit_16_19 = 0xF;		// 4GB limit

	// TSS selector
	gdt[5].sel_sys.bits.desc_limit_0_15 = 0;		// 4GB limit
	gdt[5].sel_sys.bits.desc_limit_16_19 = 0;		// 4GB limit
	gdt[5].sel_sys.bits.desc_type = SELTYPE_TSS32;	// TSS
	gdt[5].sel_sys.bits.desc_present = 1;

	// Load the GDT and all segment registers
	__asm__ __volatile__ (
		"subl $8,%%esp\n"
		"movw %%ax,2(%%esp)\n"		// Limit
		"movl %%edx,4(%%esp)\n"		// Base
		"lgdt 2(%%esp)\n"			// Load GDT
		"ljmp $0x08,$0f\n"			// Load CS
		"0:\n"
		"pushl $0x10\n"				// Data selector
		"movl (%%esp),%%ds\n"		// Load all segment registers
		"movl (%%esp),%%es\n"
		"movl (%%esp),%%fs\n"
		"movl (%%esp),%%gs\n"
		"movl (%%esp),%%ss\n"
		"addl $12,%%esp\n"			// Clean up stack
		:
		: "a" (sizeof(gdt)-1), "d" (gdt)
		: "cc"
	);
}

void gdt_set_tss(void *tss, unsigned size)
{
	unsigned base = (unsigned)tss;

	gdt[5].sel_sys.bits.desc_base_0_15 = base & 0xFFFF;
	gdt[5].sel_sys.bits.desc_base_16_23 = (base >> 16) & 0xFF;
	gdt[5].sel_sys.bits.desc_base_24_31 = (base >> 24) & 0xFF;
	gdt[5].sel_sys.bits.desc_limit_0_15 = size & 0xFFFF;
	gdt[5].sel_sys.bits.desc_limit_16_19 = size & 0xF;

	// Load task register
	__asm__ __volatile__ (
		"ltr %%ax\n"
		:
		: "a" (5 << 3)
	);
}
