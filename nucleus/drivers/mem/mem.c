/*
* Memory Management									*
* v0.01, 24.12.2003 TDS (X-Mas)						*
*		- basic routines like kmalloc, kfree, etc.	*
* v0.02, 30.03.2004 TDS								*
*		- fixed some bugs							*
* v0.03, 12.04.2004 TDS								*
*		- free, malloc, mem_walk are working :-)	*
* v0.03b, 13.05.2004 TDS							*
*		- fixed bug in malloc						*
* v0.04, 26.05.2004 TDS								*
*		- fixed bugs with large memory				*
*		- fixed bug in mem walk algorithms			*
* v0.05, 29.08.2004 Doug Gale                       *
*       - Rewrote memory size detection             *
*       - Integrated support for page tables        *
*       - Adapts to large memory (1GB+)             *
*		- Added functions to allocate single pages	*
*		- Added word and dword memset variants		*
*/

#include <drivers/mem/mem.h>
#include <nucleus.h>
#include <support.h>
#include <stdio.h>
#include <string.h>
#include <pagetbl.h>

//#define DEBUG_MEM

#define PAGE_SIZE 			4096u
#define PAGE_SHIFT 			12

#if (1<<PAGE_SHIFT) != PAGE_SIZE
#error Incorrect constant. PAGE_SIZE != (1<<PAGE_SHIFT)
#endif

unsigned *kernel_buffer;		// around (2MB / PAGE_SIZE)
								// 1MB for memory paging
char mm_initialized = 0;		// only do something if set

#define MM_ERROR_NOT_INITIALIZED	-1
#define MM_ERROR_PAGE_COUNT			-2
#define MM_ERROR_PAGE_NOT_ALLOCATED	-3
#define MM_ERROR_PAGE_NOT_USED		-4
#define MM_ERROR_PAGE_NOT_AVAILABLE	-5

#define MM_FREE			0x00
#define MM_USED			0x01
#define MM_KERNEL		0x02
#define MM_MOVEABLE		0x04
#define MM_CHANGED		0x08
#define MM_SWAP			0x10

#define size_t dword

struct
{
	size_t free, used, size;	
	size_t kernel_pages, kernel_start;	// kernel
	size_t kernel_pages_free, kernel_pages_used;
	size_t user_pages, user_start;		// user
	size_t user_pages_free, user_pages_used;
	int mm_error;
} mm_stat;

void mm_write_last_error(int write_if_none);

void memset(void *dest, int c, dword count)
{
#if 1
	__asm__ __volatile__ (
		"cld\n"
		"rep\n"
		"stosb\n"
		: "=D" (dest), "=c" (count)
		: "0" (dest), "1" (count), "a" (c)
		: "cc"
	);
#else
	dword size32	= count >> 2;
	dword fill		= (c & 0xFF) * 0x01010101;
	dword *dest32	= (dword*)dest;

	switch(count & 3)
	{
	case 3: ((byte*)dest)[count - 3] = c;
	case 2: ((byte*)dest)[count - 2] = c;
	case 1: ((byte*)dest)[count - 1] = c;
	}
	while( size32-- > 0 )
		*(dest32++) = fill;
#endif
}  

// =========================================================================
// Standard C memory functions
// =========================================================================

// Fill 16-bit values. Count * 2 bytes are filled.
void memsetw(void *dest, int c, dword count)
{
#if 1
	__asm__ __volatile__ (
		"cld\n"
		"rep\n"
		"stosw\n"
		: "=D" (dest), "=c" (count)
		: "0" (dest), "1" (count), "a" (c)
		: "cc"
	);
#else
	word *dest16 = dest;
	while (count--)
		*dest16++ = c;
#endif
}

// Fill 32-bit values. Count * 4 bytes are filled.
void memsetd(void *dest, int c, dword count)
{
#if 1
	__asm__ __volatile__ (
		"cld\n"
		"rep\n"
		"stosl\n"
		: "=D" (dest), "=c" (count)
		: "0" (dest), "1" (count), "a" (c)
		: "cc"
	);
#else
	dword *dest32 = dest;
	while (count--)
		*dest32++ = (dword)c;
#endif
}

void memcpy(void * dest, const void * src, dword count)
{
	dword size32	= count >> 2;
	dword *dest32	= (dword*)dest;
	dword *src32	= (dword*)src;

	switch( (count-(size32<<2)) )
	{
		case 3:((byte*)dest)[count-3] = ((byte*)src)[count-3];
		case 2:((byte*)dest)[count-2] = ((byte*)src)[count-2];
		case 1:((byte*)dest)[count-1] = ((byte*)src)[count-1];
	}
	while( size32-- > 0 )
		*(dest32++) = *(src32++);
}

void memmove(void *dest, const void *src, dword count)
{
	dword size32	= count>>2,i;
	dword *dest32	= (dword*)dest;
	byte  *dest8;
	dword *src32	= (dword*)src;
	byte  *src8;

	if( dest > src )
	{
		dest8 = (byte*)dest;
		src8 = (byte*)src;
		switch( (count-(size32<<2)) )
		{
		case 3:
			dest8[count-1] = src8[count-1];
			dest8[count-2] = src8[count-2];
			dest8[count-3] = src8[count-3];
			break;
		case 2:
			dest8[count-1] = src8[count-1];
			dest8[count-2] = src8[count-2];
			break;
		case 1:
			dest8[count-1] = src8[count-1];
			break;
		}
		while( size32-- )
			dest32[size32] = src32[size32];
	}
	else
	{
		for(i=0;i<size32;i++)
			*(dest32++) = *(src32++);

		dest8 = (byte*)dest32;
		src8 = (byte*)src32;
		switch( (count-(size32<<2)) )
		{
		case 3:*dest8++ = *src8++;
		case 2:*dest8++ = *src8++;
		case 1:*dest8++ = *src8++;
		}
	}
}

// =========================================================================
// Kernel page allocation functions
// =========================================================================

static unsigned int BLOCK(dword x)
{
	return (dword)(x-mm_stat.kernel_start) / PAGE_SIZE;
}

static dword ADDR(unsigned int x)
{	
	return (dword)(x*PAGE_SIZE) + mm_stat.kernel_start;
}

static int mm_get_page(int count, int * block)
{
	unsigned int i, j;	
	
	if (!mm_initialized) return MM_ERROR_NOT_INITIALIZED;
	if (count < 1) return MM_ERROR_PAGE_COUNT;
	for (i=0; i < mm_stat.kernel_pages; i++)
	{
		if (kernel_buffer[i] & MM_USED) continue;	// check for used bit
		for (j=0; j < count; j++)
		{
			if (kernel_buffer[i+j] & MM_USED)		// are there enough pages
				break;
		}
		if (j == count)
		{
			for (j=0; j < count; j++)
				kernel_buffer[i+j] |= MM_USED;				// set used bit
			mm_stat.kernel_pages_free -= count;
			mm_stat.kernel_pages_used += count;
			*block = i;
			return count;
		}
	}
	return MM_ERROR_PAGE_NOT_ALLOCATED;
}

static int mm_free_page(int page)
{
	if (page >= 0 && page < mm_stat.kernel_pages)
	{
		if (kernel_buffer[page] & 1)
		{
				kernel_buffer[page] &= ~1;	// set free bit
				mm_stat.kernel_pages_free++;
				mm_stat.kernel_pages_used--;
				return 0;
		}
		else	
		{
			return MM_ERROR_PAGE_NOT_USED;
		}
	}
	else	
	{
		return MM_ERROR_PAGE_NOT_AVAILABLE;
	}
}

// Allocate a single entire page
// Returns zero on error
void *mm_page_alloc(void)
{
	unsigned i;
	void *p;

	if (!mm_initialized)
		return 0;

	// Search for an available page
	for (i=0; i < mm_stat.kernel_pages; i++)
	{
		if (!(kernel_buffer[i] & MM_USED))
			break;
	}
	if (i >= mm_stat.kernel_pages)
		return 0;

	// Mark page as used
	kernel_buffer[i] |= MM_USED;

	// Calculate pointer to the page
	p = (void *)ADDR(i);

	// Fill the page with garbage
	memsetd(p, 0xcccccccc, PAGE_SIZE >> 2);

	mm_stat.kernel_pages_free--;
	mm_stat.kernel_pages_used++;

	// Return pointer to the page
	return p;
}

// Free a single entire page
void mm_page_free(void *page)
{
	unsigned i;

	if (!mm_initialized) return;

	i = BLOCK((dword)page);

	// Debug. Panic if page is already free.
	if (!(kernel_buffer[i] & MM_USED))
		panic();

	// Mark page as free
	kernel_buffer[i] &= ~MM_USED;

	// Fill the page with garbage
	memsetd(page, 0xcdcdcdcd, 1024);

	mm_stat.kernel_pages_free++;
	mm_stat.kernel_pages_used--;
}

unsigned long mm_get_memsize()
{
	return mm_stat.size;
}

static const unsigned int SIZE = 4;

void * kmalloc(dword size)
{
	unsigned int block, pages;
	dword addr;

	if (!mm_initialized) return 0;
	size += SIZE;
	pages = (size % PAGE_SIZE) ? 1 : 0;		// adjust one extra page
	pages += (size / PAGE_SIZE);			// calculate needed pages
	if ((mm_stat.mm_error = mm_get_page(pages, &block)) > 0)
	{
		addr = ADDR(block);
#ifdef DEBUG_MEM		
		dprintf("memory: Allocated %ld Bytes (%d pages) at 0x%lX\n", size-SIZE, pages, addr);
#endif		
		*(word *)addr = pages;
		return (void *)(addr + SIZE);
	}
	return 0;
}

void kfree(void * ptr)
{
	int i;
	
	if (!mm_initialized) return;
	
	dword addr = (dword)((ptr) - SIZE);
	unsigned int pages = *(unsigned int *)addr;
	unsigned int startpage = BLOCK(addr);
	
#ifdef DEBUG_MEM
	dprintf("memory: freeing memory (%d pages, 0x%lX)\n", pages, addr);
#endif
	for (i=startpage; i < startpage + pages; i++)
	{
		if (mm_free_page(i) != 0)
		{
			mm_write_last_error(0);
			ptr = NULL;	// wasted memory
			break;
		}
	}
	ptr = NULL;
}

void mm_write_last_error(int write_if_none)
{
	switch(mm_stat.mm_error)
	{
		case MM_ERROR_NOT_INITIALIZED:		
				dprintf("memory: error: Memory management not initialized!\n"); break;
		case MM_ERROR_PAGE_COUNT:			
				dprintf("memory: error: Wrong page count!\n"); break;
		case MM_ERROR_PAGE_NOT_ALLOCATED:	
				dprintf("memory: error: Couldn't allocate pages!\n"); break;
		case MM_ERROR_PAGE_NOT_USED:
				dprintf("memory: error: Page not used!\n"); break;
		case MM_ERROR_PAGE_NOT_AVAILABLE:
				dprintf("memory: error: Page not available!\n"); break;
		default: if (write_if_none) dprintf("memory: No error\n");
	}
}

// user stuff
// double linked list for easy use

typedef struct MEM_LIST
{
	char status;	// free, used, kernel (driver, etc.), moveable, changed, swap
	size_t size;
	struct MEM_LIST *next;
} MEM_LIST;	

static MEM_LIST *ml, *ml_start;

static int mem_create_user(size_t entries)
{
	MEM_LIST *tmp, *next;
	
	if (!mm_initialized) return MM_ERROR_NOT_INITIALIZED;
	// reserve kernel space, max. 320kb for allocating 32768 pages à 4096bytes (4GB)
	// checking if space is there, else )!/$&§/§§" :-)
    dprintf("memory: Allocating user memory space (%ld entries, %ld bytes)...", 
			entries, (dword)(entries*sizeof(MEM_LIST)));
    ml = (MEM_LIST *)kmalloc(entries*sizeof(MEM_LIST));
    if (!ml)
    {
	    printf("memory: kernel panic: couldn't allocate user space...\n");
	    printf("        running in bogus mode...");
		halt(1);
	    return -1;	// shouldn't come here
    }   
    dprintf("OK\n");
    if (ml)
    {
    	ml->status = MM_FREE;
    	ml->next = ml;
    	ml_start = ml;
    	while (entries--)
    	{
		    tmp = (MEM_LIST *)((dword)ml + sizeof(MEM_LIST));
	    	if (!tmp)
	    	{
			    mm_write_last_error(0);			    
		    	return -1;
			}
	   		tmp->status = MM_FREE;
	   		tmp->size = 0;
	    	next = ml->next;
	   		ml->next = tmp;
	   		tmp->next = next;
	   		ml = ml->next;
		}
		return 0;
	}
	return -1;
}

void * malloc(size_t size)
{
	size_t pages, counter;
	dword found;
	void * mem_addr;

	if (!mm_initialized) return NULL;
	
	size += SIZE;
	pages = (size % PAGE_SIZE) ? 1 : 0;		// adjust one extra page
	pages += (size / PAGE_SIZE);			// calculate needed pages
	
	counter = 0;
	found = 0;
#ifdef DEBUG_MEM
	dprintf("memory: Trying to allocate %ld user pages.\n", pages);
#endif
	ml = ml_start;
	while (1)
	{
		found++;
		if (ml->status != MM_FREE)
			found = 0;
		else
		if (found == pages)
			break;		
		ml = ml->next;
		counter++;
		if (ml == ml_start) break;
	}
	if (found == pages)
	{
		ml = ml_start;
#ifdef DEBUG_MEM
		dprintf("memory: user: Allocated %ld pages at %lX\n", pages, 
				(dword)(mm_stat.user_start + (counter*PAGE_SIZE)));
#endif		
		mem_addr = (void *)(mm_stat.user_start + (counter * PAGE_SIZE));
		while(counter--)
			ml = ml->next;
		ml->size = pages;
		while (pages--)
			ml->status = MM_USED;
		return mem_addr;
	}
#ifdef DEBUG_MEM
	dprintf("memory: user: Couldn't allocate %ld pages\n", pages);
#endif		
	return 0;
}

void free(void * ptr)
{	
	if (!mm_initialized) return;
	
	dword start = (dword)(ptr - mm_stat.user_start) / PAGE_SIZE;
	
#ifdef DEBUG_MEM
	dprintf("memory: freeing user memory (0x%lX)\n", (dword)(ptr));
#endif	
	ml = ml_start;
	while (start--)	ml = ml->next;
	start = ml->size;
	while (start-- > 1)
	{
		ml->status = MM_FREE;
		ml->size = 0;
		ml = ml->next;
	}
	ptr = NULL;
}

// sollte eigentlich funktionieren.
void mem_walk(void)
{
	size_t mfree, mused, melse;

	mfree = 0;	mused = 0;	melse = 0;

	ml = ml_start;
	while (1)
	{
		if (ml->status == MM_USED)
			mused++;
		else
		if (ml->status == MM_FREE)
			mfree++;
		else
			melse++;
		ml = ml->next;
		if (ml == ml_start) break;
	}
	printf("memory:\n");
	printf("\t%ldKB (%ldMB) free, %ldKB(%ldMB) used, %ldKB(%ldMB) other, %ld pages\n",
		(mfree * PAGE_SIZE) / 1024, (((mfree * PAGE_SIZE) / 1024) / 1024),
		(mused * PAGE_SIZE) / 1024, (((mused * PAGE_SIZE) / 1024) / 1024),
		(melse * PAGE_SIZE) / 1024, (((melse * PAGE_SIZE) / 1024) / 1024),
		mfree+mused+melse);
	printf("\t%ldKB (%ldMB) kernel free, %ldKB(%ldMB) kernel used\n",
		(mm_stat.kernel_pages_free * PAGE_SIZE) / 1024, (((mm_stat.kernel_pages_free * PAGE_SIZE) / 1024) / 1024),
		(mm_stat.kernel_pages_used * PAGE_SIZE) / 1024, (((mm_stat.kernel_pages_used * PAGE_SIZE) / 1024) / 1024));
}

static void mm_show(void)
{
	dprintf("memory: Size: %ldKB, Available %ldKB\n", mm_stat.size / 1024, mm_stat.free / 1024);
	dprintf("        Kernel: %ldKB\n", mm_stat.kernel_pages * 4);
	dprintf("	       starts at 0x%lX (%ldKB)\n", mm_stat.kernel_start, mm_stat.kernel_start / 1024);
	dprintf("	       %ld pages (%ld free)\n", mm_stat.kernel_pages, mm_stat.kernel_pages_free);
	dprintf("        User  : %ldKB\n", mm_stat.user_pages * 4 );
	dprintf("	       starts at 0x%lX (%ldKB)\n", mm_stat.user_start, mm_stat.user_start / 1024);
	dprintf("	       %ld pages (%ld free)\n", mm_stat.user_pages, mm_stat.user_pages_free);
}

// Detect memory size
dword detect_memory(void)
{
	volatile unsigned *zero, oldzero, *mem, oldmem;
	unsigned long result;
	int ints_were_enabled;
	extern char __bss_en[];

	if (mm_initialized)
		return mm_stat.size;

	printf("memory: Detecting physical memory size...");

	// Disable interrupts
	ints_were_enabled = interrupts_disable();

	// Pointer to beginning of memory
	zero = (unsigned*)0x00000000;
	oldzero = *zero;

	// Put 0x76543210 at address zero for wraparound detection
	*zero = 0x76543210;

	// Start search at end of kernel, so I don't overwrite myself!
	mem = (unsigned*)(((unsigned)__bss_en + 65535) & ~65535);

	// Loop through memory
	do {
		// Save content of memory and store a value
		oldmem = *mem;
		*mem = 0x01234567;

		// Detect wraparound
		if (*zero != 0x76543210)
			break;		// Detected wraparound

		// Detect memory not responding
		if (*mem != 0x01234567)
			break;

		// Restore old memory content
		*mem = oldmem;

		// Step to next 64KB
		mem = (unsigned*)((unsigned)mem + 65536);
	} while (1);

	// Restore overwritten memory at zero
	*zero = oldzero;

	if (ints_were_enabled)
		interrupts_enable();

	result = (unsigned)mem;

	printf("%dKB (%dMB)\n", result >> 10, result >> 20);

	return result;
}

/*
Physical memory map:
|---------------------------------------------------------------------------|
|                                 User heap                                 |
|---------------------------------------------------------------------------|
|                                 Kernel heap (2MB)                         |
|---------------------------------------------------------------------------|
|                                 Kernel page directory and page tables     |
|===========================================================================|
| 0x00100000-           (  ???KB) Kernel image (code, data, bss)            |
|===========================================================================|
| 0x000F0000-0x000FFFFF (   64KB) BIOS ROM                                  |
| 0x000C8000-0x000EFFFF (  160KB) Unused adapter ROM area                   |
| 0x000C0000-0x000C7FFF (   64KB) Video ROM                                 |
| 0x000B8000-0x000BFFFF (   32KB) Text mode video RAM                       |
| 0x000B0000-0x000B7FFF (   32KB) Unused                                    |
| 0x000A0000-0x000AFFFF (   64KB) VGA graphics video RAM                    |
| 0x00090000-0x0009FFFF (   64KB) DMA buffer (currently only used by floppy |
| 0x00050000-0x0008FFFF (  256KB) Kernel stack                              |
| 0x0004F000-0x0004FFFF (    4KB) Kernel stack guard page                   |
| 0x00001000-0x0004EFFF (   312B) Reserved for ISA DMA operations           |
| 0x00000000-0x00000FFF (    4KB) Guard page for null dereference detection |
|---------------------------------------------------------------------------|

  The kernel page tables map all physical memory with no translation.

  A bitmap is used for allocation of pages after the kernel image.
*/

// Calculate initial memory map. Adapts to physical memory size.
static int mm_init_memmap(void)
{
	extern char __bss_en[];
	int pagetbl_size;
	unsigned mem_begin;

	// Get physical memory size
	mm_stat.size = detect_memory();

	// Must have 8MB
	if (mm_stat.size < (1<<23))
		return 0;

	// End of kernel, round up to next page
	mem_begin = ((unsigned)__bss_en + 0x00000FFF) & 0xFFFFF000;

	printf("Top of kernel: %d\n", mem_begin);

	// Set page table base address
	page_dir = (void*)mem_begin;

	// Calculate amount of memory needed to map entire physical memory
	pagetbl_size = mm_stat.size >> 22;	// Number of page tables needed
	pagetbl_size++;						// One more page for page directory
	pagetbl_size <<= PAGE_SHIFT;				// Bytes needed for all page tables
	printf("Page tables: %d\n", pagetbl_size);

	// Calculate how large the kernel memory space needs to be
	mm_stat.kernel_pages = mm_stat.size;		// Mem range in bytes
	mm_stat.kernel_pages >>= PAGE_SHIFT;				// Convert to pages
	mm_stat.kernel_pages *= sizeof(MEM_LIST);		// One structure per page
	mm_stat.kernel_pages += PAGE_SIZE - 1;			// Round up
	mm_stat.kernel_pages >>= PAGE_SHIFT;				// Convert to pages

	mm_stat.kernel_pages_free = mm_stat.kernel_pages;

	// Calculate the beginning of the kernel heap
	mm_stat.kernel_start = mem_begin + pagetbl_size;

	printf("Kernel start: %d\n", mm_stat.kernel_start);

	// Calculate the beginning and size of the user heap
	mm_stat.user_start = mm_stat.kernel_start + (mm_stat.kernel_pages * PAGE_SIZE);

	printf("User start: %d\n", mm_stat.kernel_start);

	mm_stat.user_pages = (mm_stat.size - mm_stat.user_start) / PAGE_SIZE;

	mm_stat.user_pages_free = mm_stat.user_pages;

	printf("User pages: %d\n", mm_stat.user_pages);

	return 1;
}

void mm_init(void)
{
	unsigned int i;

	if (!mm_init_memmap()) {
		printf("memory: At least 8MB required!\n");
		printf("        Aborting...\n");
		halt(1);
	}

	if (mm_stat.user_pages < 256)
	{
		printf("memory: %ld pages of at least 256 pages could be created.\n", mm_stat.user_pages);
		printf("        Aborting...\n");
		halt(1);
	}	
	dprintf("memory: %ldKB found (using %ldKB)\n", (dword)(mm_stat.size / 1024), (dword)(mm_stat.free / 1024));
	dprintf("memory: Allocating kernel pages...");
	for (i=0; i < mm_stat.kernel_pages; i++)
		kernel_buffer[i] = 0;	// marked as free
	dprintf("%ld pages allocated.\n", mm_stat.kernel_pages);
	// create linked list
	mm_initialized = 1;
	mem_create_user(mm_stat.user_pages);	// create list with user memory from the kernel memory (max 320kb)
	mm_show();

	// Initialize paging
	pagetbl_init();
}

