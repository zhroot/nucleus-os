// nucload.c

// C loader runs in 16-bit mode.
// Sufficient to load and run the kernel

// Generates 32-bit instructions with all prefixes necessary for them
// to run in 16-bit mode

// Header stub
asm (
	".code16gcc\n"
	".section .head\n"
	"ljmp $0,$0f\n"
	"0:\n"
	"cli\n"
	"movl %cs,%eax\n"
	"movl %eax,%ds\n"
	"movl %eax,%es\n"
	"movl %eax,%fs\n"
	"movl %eax,%gs\n"
	"movl %eax,%ss\n"
	"movl $___stack_bottom,%esp\n"
	"movl $___bss_en,%ecx\n"
	"movl $___bss_st,%edi\n"
	"subl %edi,%ecx\n"
	"movb $0,%al\n"
	"cld\n"
	"rep stosb\n"
	"movl %edx,_boot_drive\n"
	"sti\n"
	"call _main\n"
//	".fill 10000,1,0x90\n"
	"int $0x20\n"
	".section .text\n"
);

#define DEBUG	0

extern unsigned char footer_sig;

// Register contents on startup
unsigned boot_drive;

// Don't make symbols static when debugging so I can see where everything
// is in the map file
#if DEBUG
#define STATIC
#else
#define STATIC static
#endif

#include <stdarg.h>

#define KERNEL_FILENAME		"NUCLEUS.BIN"

typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;

// Boot sector
typedef struct tagBootSector {
	byte jmpinsn[3];					// JMP instruction
	byte oemid[8];						// OEM ID
	word bytespersect;					// Bytes per sector
	byte sectsperclust;					// Sectors per cluster
	word rsvdsect;						// Reserved sectors
	byte totalfats;						// Total FATs
	word maxrootentries;				// Max root dir entries
	word totalsectssmall;				// Total sectors
	byte mediadescriptor; 				// Media descriptor
	word sectsperfat;					// Sectors per fat
	word sectspertrack;					// Sectors per track
	word numheads;						// Number of heads
	dword hiddensects;					// Hidden sectors
	dword totalsectslarge;				// Large sector count
	byte drivenumber;					// Drive number
	byte flags;							// Flags
	byte signature;						// Signature
	dword volumeid;						// Volume ID
	byte volumelabel[11];				// Volume label
	byte systemid[8];					// System ID
} __attribute__((packed)) BootSector;

// Directory entry
typedef struct tagDirEntry {
	byte file_name[8];					// Base name
	byte file_ext[3];					// Extension
	byte file_attr;						// Attributes
	byte file_lc;						// "Lowercase" flag
	byte file_ctime_ms;					// Creation time milliseconds
	word file_ctime;					// Creation time
	word file_cdate;					// Creation date
	word file_adate;					// Accessed date
	word file_reserved;					// Reserved
	word file_time;						// Modification time
	word file_date;						// Modification date
	word file_cluster;					// Starting cluster
	dword file_size;					// File size
} __attribute__((packed)) DirEntry;

STATIC BootSector bootsect;
STATIC DirEntry *pDirStart, *pDirEnd;

// Symbols from linker script
extern char __load_buffer_1KB[];
extern char __allocator_start[];

// Dynamic memory allocator
STATIC char *pAlloc = __allocator_start;
// Root directory (dynamically allocated)
STATIC DirEntry *pRoot;
// FAT (dynamically allocated)
STATIC word *pFat;

STATIC word nRootBase, nRootSects, nRootBytes;
STATIC word nFatBase, nFatBytes, nFatSects, nFatsSects, nDataBase;

// Print a character
STATIC void PrintChar(byte c)
{
	__asm__ __volatile__ (
		"pushal\n"
		"int $0x10\n"
		"popal\n"
		:
		: "a" ((dword)c | 0x0e00), "b" (0x0007)
		: "cc"
	);
}

// Print the passed string
void PrintString(byte *str)
{
	while (*str)
		PrintChar(*str++);
}

// Lookup table for hex conversions
STATIC char hexlookup[] = "0123456789ABCDEF";

// Minimal printf implementation for debugging
STATIC void Printf(char *pFormat, ...)
{
	char *pBegin = pFormat;
	byte num[12], *pn;
	dword base, n;
	va_list ap;

	va_start(ap, pFormat);

	num[10] = 0;

	while (*pBegin) {
		switch (*pBegin) {
		case '%':
			switch (pBegin[1]) {
			case 'd':
				base = 10;
				n = va_arg(ap,int);
				break;

			case 'x':
				base = 16;
				n = va_arg(ap,int);
				break;

			case 's':
				base = 0;
				n = 0;
				pn = va_arg(ap, byte *);
				break;

			}

			if (base) {
				if (n == 0) {
					pn = (base == 16 ? "00000000" : "0");
				} else {
					pn = num+10;
					while (n) {
						*--pn = hexlookup[n % base];
						n /= base;
					}
					if (base == 16) {
						while (pn >= num+3) {
							*--pn = '0';
						}
					}
				}
			}
			pBegin += 2;
			PrintString(pn);
			break;

		case '\n':
			PrintString("\r\n");
			pBegin++;
			break;

		default:
			PrintChar(*pBegin++);
			break;
		}
	}

	va_end(ap);
}

// Read the specified number of sectors from LBA sector number
STATIC word ReadSectors(void *dest, dword block, word count)
{
	word n, c, h, s;

//	Printf("Read sectors: dest=0x%x, block=0x%x, count=0x%x\n", dest, block, count);

	while (count--) {
		// Sanity
		if (block >= bootsect.totalsectssmall) {
			PrintString("\nSector out of range!\n");
			return 0;
		}

		s = block % bootsect.sectspertrack;
		n = block / bootsect.sectspertrack;
		h = n % bootsect.numheads;
		c = n / bootsect.numheads;
		if (!ReadCHS(dest, bootsect.drivenumber, c, h, s + 1))
			return 0;

		dest += bootsect.bytespersect;
		block++;
	}

	return 1;
}

// Translate from cluster number to LBA sector number and read sector
STATIC word ReadCluster(void *dest, word cluster)
{
	word c = (cluster - 2) * bootsect.sectsperclust + nDataBase;
	return ReadSectors(dest, c, bootsect.sectsperclust);
}

// Decode the specified cluster of the fat
STATIC word ReadFat(word idx)
{
	word i, odd, v;
	odd = !!(idx & 1);
	i = idx + (idx >> 1);
	v = *(word*)((byte*)pFat + i);
	return (odd) ? (v >> 4) : (v & 0x0fff);
}

// Search the directory for the specified filename
STATIC DirEntry *FindDirEnt(char *pFilename)
{
	char aName[12];
	word i;
	DirEntry *p;
	for (i = 0; i < sizeof(aName); i++)
		aName[i] = ' ';
	aName[11] = 0;

	i = 0;
	while (*pFilename) {
		if (*pFilename == '.') {
			i = 8;
			pFilename++;
		} else
			aName[i++] = *pFilename++;
	}

	for (p = pDirStart; p < pDirEnd; p++) {
		for (i = 0; i < 11; i++)
			if (aName[i] != p->file_name[i])
				break;
		if (i >= 11)
			break;
	}
	return p < pDirEnd ? p : 0;
}

// Returns true on success
word LoadImage(DirEntry *p)
{
	dword progress = 0;
	dword dest = 0x00100000;
	word cluster = p->file_cluster;
	word bytespercluster = bootsect.sectsperclust * bootsect.bytespersect;
	while (cluster != 0x0FFF) {
//		Printf("Cluster: 0x%x\n", cluster);
		if ((progress++ & 31) == 0) {
			Printf("\rLoading kernel...%dKB", (progress * bytespercluster) >> 10);
		}
		if (!ReadCluster(__load_buffer_1KB, cluster))
			break;
		CopyToExt(__load_buffer_1KB, dest, bytespercluster);
		dest += bytespercluster;

		cluster = ReadFat(cluster);
	}
	Printf((cluster == 0x0FFF) ? " OK\n" : " ERROR\n");
	return cluster == 0x0FFF;
}

// Naming convention:
//  n???Base = LBA sector number of beginning
//  n???Bytes = Size of data structure in bytes
//  n???Sects = Count of sectors
// and ??? is
// Root = Root directory
// Fat = Main fat
// Fats = All fats combined

void Halt()
{
	__asm__ __volatile__ (
		"L1:"
		"cli\n"
		"hlt\n"
		"jmp L1\n"
	);
}

STATIC dword GetFreeMemory()
{
	dword esp;
	__asm__ __volatile__ ("movl %%esp,%%eax"
		: "=a" (esp)
	);
	return esp - (dword)pAlloc;
}

STATIC byte WaitKey()
{
	byte key;
	__asm__ __volatile__ (
		"int $0x16\n"
		: "=a" (key)
		: "a" (0x0000)
	);
	return key;
}

int main()
{
	int i;
	dword *p;
	DirEntry *pDirEnt;

	if (footer_sig != 0xE0) {
		PrintString("FAILED! LOADER.SYS trucated. \r\n");
		Halt();
	}

	Printf("Boot drive: 0x%x\n", boot_drive);

	// Load boot sector
	PrintString("Reading boot sector...");
	if (!ReadCHS(&bootsect, bootsect.drivenumber, 0, 0, 1)) {
		PrintString("FAILED!\r\n");
		Halt();
	}
	PrintString("OK\r\n");

//	Printf("Total sectors: %d\n", bootsect.totalsectssmall);
//	Printf(" Sectors size: %d\n", bootsect.bytespersect);
//	Printf("   Total FATs: %d\n", bootsect.totalfats);

	nFatBase = bootsect.rsvdsect;

	// Calculate size of one fat (FAT12)
	nFatBytes = bootsect.totalsectssmall +
			(bootsect.totalsectssmall >> 1);
	nFatSects = (nFatBytes + bootsect.bytespersect - 1) / 
			bootsect.bytespersect;
	// Calculate total size of all fats in sectors
	nFatsSects = nFatSects * bootsect.totalfats;

	// Calculate base of root
	nRootBase = nFatBase + nFatsSects;

	// Find root directory
	nRootSects = 32 * bootsect.maxrootentries / bootsect.bytespersect;
	nRootBytes = nRootSects * bootsect.bytespersect;

	// Calculate base sector of data area
	nDataBase = nRootBase + nRootSects;

	// Allocate memory for root directory
	pDirStart = (void*)pAlloc;
	pAlloc += nRootBytes;
	pDirEnd = (void*)pAlloc;

	pFat = (void*)pAlloc;
	pAlloc += nFatBytes;

//	Printf("     pFat: 0x%x\n", pFat);
//	Printf("pDirStart: 0x%x\r\n", pDirStart);

	// Load the root directory
	PrintString("Reading directory...");
	if (!ReadSectors(pDirStart, nRootBase, nRootSects)) {
		PrintString("FAILED!");
		Halt();
	}
	PrintString("OK\r\n");

	// Load the FAT
	PrintString("Reading file allocation table...");
	if (!ReadSectors(pFat, nFatBase, nFatSects)) {
		PrintString("FAILED!");
		Halt();
	}
	PrintString("OK\r\n");

	// Find directory entry
	PrintString("Searching for kernel...");
	pDirEnt = FindDirEnt(KERNEL_FILENAME);
	if (!pDirEnt) {
		PrintString("FAILED!");
		Halt();
	}
	PrintString("OK\r\n");

	// Load the image
	PrintString("Loading kernel...");
	if (!LoadImage(pDirEnt)) {
		PrintString("FAILED!");
		Halt();
	}
	PrintString("OK\r\n");

	PrintString("Entering kernel...\r\n");

	// Enter kernel
	EnterKernel();

	return 0;
}
