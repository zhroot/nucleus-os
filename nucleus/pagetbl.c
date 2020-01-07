// pagetbl.c

// This code manages the paging tables.

// v0.1: Doug Gale
//	- Initial revision
//	- Maps entire physical memory using identity mapping
//	- Applies protection to hardware areas
//	- Marks zero page not present to catch null-pointers
//	- Added code to conveniently manipulate page protections
// v0.2: Doug Gale
//	- Added provision for multiple page directories

#include <pagetbl.h>
#include <drivers/mem/mem.h>
#include <support.h>
#include <stdio.h>

// Page table entry
union tagPageTblEnt {
	// Bitfield for convenient access
	struct {
		unsigned pg_present:1;		// Page is present
		unsigned pg_writable:1;		// Page is writable
		unsigned pg_user:1;			// Page only accessible in user mode
		unsigned pg_writethru:1;	// Don't cache writes
		unsigned pg_nocache:1;		// Don't cache at all
		unsigned pg_accessed:1;		// Has been read
		unsigned pg_modified:1;		// Has been written
		unsigned pg_4meg:1;			// 4MB page (only in page directory)
		unsigned pg_global:1;		// Don't flush this entry
		unsigned pg_avail:3;		// Available for OS use
		unsigned pg_base:20;		// High 20 bits of phys addr of page
	} bits;

	// Access as a raw 32-bit value
	unsigned long dw;
};

// One page is allocated for the page directory.
// Each entry of the page directory points to the page table for that region
// Each page table is an array of physical addresses for that page.

// Pointer to page directory
// This is set in drivers/mem/mem.c
PageTblEnt *page_dir;

// Initialize page tables and enable paging
void pagetbl_init()
{
	extern char __text_st[];
	extern char __text_en[];
	unsigned memsize, i, addr, j;
	PageTblEnt *alloc, *tbl;
	PageTblEnt te;

	// Get ready to allocate pages
	alloc = page_dir + 1024;

	// Clear page directory
	memsetd(page_dir, 0, 1024);

	// Get size of physical memory
	memsize = mm_get_memsize();

	// Allocate pages for page tables. One page for each 4 megabytes.
	// Fill in the page directory so I don't need to store separate pointers
	// Walk through memory 4 megabytes at a time
	for (i = 0, addr = 0; addr < memsize; i++, addr += (1<<22)) {
		// Allocate a page for page table
		tbl = alloc;
		alloc += 1024;

		// Clear new page table
		memsetd(tbl, 0, 1024);

		//
		// Fill page directory entry
		//

		// Pointer to page table
		page_dir[i].bits.pg_base = (unsigned long)tbl >> 12;

		page_dir[i].bits.pg_present = 1;
		page_dir[i].bits.pg_writable = 1;
//		page_dir[i].bits.pg_user = 0;
//		page_dir[i].bits.pg_writethru = 0;
//		page_dir[i].bits.pg_nocache = 0;
//		page_dir[i].bits.pg_accessed = 0;
//		page_dir[i].bits.pg_modified = 0;
//		page_dir[i].bits.pg_4meg = 0;
//		page_dir[i].bits.pg_global = 0;
//		page_dir[i].bits.pg_avail = 0;

		//
		// Fill page table
		//

		for (j = 0; j < 1024; j++) {
			tbl[j].bits.pg_base = (j * 4096 + addr) >> 12;

			tbl[j].bits.pg_present = 1;
			tbl[j].bits.pg_writable = 1;
//			tbl[j].bits.pg_user = 0;
//			tbl[j].bits.pg_writethru = 0;
//			tbl[j].bits.pg_nocache = 0;
//			tbl[j].bits.pg_accessed = 0;
//			tbl[j].bits.pg_modified = 0;
//			tbl[j].bits.pg_4meg = 0;
//			tbl[j].bits.pg_global = 0;
//			tbl[j].bits.pg_avail = 0;
		}
	}

	//
	// Mark not-present regions
	//

	te.dw = 0;
	te.bits.pg_present = 1;		// CLEAR the present bit
	// Catch null pointer accesses
	pagetbl_chgprotect(page_dir, (void*)0x00000000, (void*)0x00000FFF, te.dw, 0);
	// Mark hardware region not present
	pagetbl_chgprotect(page_dir, (void*)0x000D0000, (void*)0x000EFFFF, te.dw, 0);

	//
	// Mark read-only regions
	//

	te.dw = 0;
	te.bits.pg_writable = 1;	// CLEAR the writable bit
	pagetbl_chgprotect(page_dir, (void*)0x000C0000, (void*)0x000FFFFF, te.dw, 0);

	// Make text section read only
	pagetbl_chgprotect(page_dir, (void*)__text_st, (void*)__text_en, te.dw, 0);

	// Disable caching of video memory entirely
	te.dw = 0;
	te.bits.pg_nocache = 1;		// SET the no-cache bit
	te.bits.pg_writethru = 1;	// SET the write-thru bit
	pagetbl_chgprotect(page_dir, (void*)0x000A0000, (void*)0x000AFFFF, 0, te.dw);

	//
	// Enable paging
	//

	__asm__ __volatile__ (
		"movl %0,%%cr3\n"
		"movl %%cr0,%0\n"
		// Enable paging, write-protect, and alignment exceptions
		"orl $0x80050000,%0\n"	
		"movl %0,%%cr0\n"
		: 
		: "r" (page_dir)
		: "0"
	);
}

// Return a pointer to the page table entry for the specified address
// Pass a pointer to the page directory to manipulate
PageTblEnt *pagetbl_findpage(PageTblEnt *dir, void *ptr)
{
	unsigned addr, diridx, tblidx;
	PageTblEnt *tbl;

	addr = (unsigned long)ptr;

	// Calculate page directory index
	diridx = addr >> 22;

	// Calculate pointer to page table
	tbl = (PageTblEnt*)(dir[diridx].bits.pg_base << 12);
	if (!tbl)
		return 0;

	// Calculate page table index
	tblidx = (addr - (diridx << 22)) >> 12;

	return tbl + tblidx;
}

// Modify the protection of the page that corresponds 
// to the given linear address(es)
// The end of the range is not inclusive
// Pass an unsigned long specifying bits to clear and bits to set
// The "clear" is performed before the "set"
// Returns the old value of the last entry modified
unsigned long pagetbl_chgprotect(PageTblEnt *dir, void *ptr, void *to, 
		unsigned long clr, unsigned long set)
{
	unsigned long ret;
	PageTblEnt *tblent;

	do {
		// Get pointer to page table entry
		tblent = pagetbl_findpage(dir, ptr);
		if (!tblent) {
			printf("memory: page not found when adjusting protection! (0x%x)\n", 
					(unsigned)ptr);
			panic();
		}

		// Save original value for return
		ret = tblent->dw;

		// Clear/set specified bits
		tblent->dw &= ~clr;
		tblent->dw |= set;

		// Go to next page
	} while ((ptr += 4096) && (ptr < to));

	return ret;
}

