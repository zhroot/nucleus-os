// pagetbl.h

// Paging tables

typedef union tagPageTblEnt PageTblEnt;

// Pointer to page directory
extern PageTblEnt *page_dir;

// Initialize page tables and enable paging
void pagetbl_init(void);

// Return a pointer to the page table entry for the specified address
PageTblEnt *pagetbl_findpage(PageTblEnt *dir, void *ptr);

// Modify the protection of the page that corresponds 
// to the given physical address(es)
// The end of the range is not inclusive
// Pass a dword specifying bits to clear and bits to set
// Returns the old value of the last entry modified
unsigned long pagetbl_chgprotect(PageTblEnt *dir, void *ptr, void *to, 
		unsigned long clr, unsigned long set);
