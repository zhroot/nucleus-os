#ifndef FAT_H
#define FAT_H

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned long int dword;

#define ATTR_READ_ONLY		0x01
#define ATTR_HIDDEN 		0x02
#define ATTR_SYSTEM 		0x04
#define ATTR_VOLUME_ID 		0x08
#define ATTR_DIRECTORY		0x10
#define ATTR_ARCHIVE  		0x20
#define ATTR_LONG_NAME 		0x0F
#define ATTR_LONG_NAME_MASK	0x40

struct bpblck // BIOS Parameter Block
{
	word BytesPerSec; // 512, 1024, 2048 or 4096 (Should be 512)
	byte SecPerCluster; // a power of 2 (Should be 1)
	word RsvdSecCnt; // not 0 (Should be 1 for fat16 and 32 for fat32)
	byte NumFATs; // not 0 (Should be 2)
	word RootEntCnt; // (RootEntCnt * 32) / BytesPerSec must be valid on fat16, for fat32 it must be 0
	word TotSec16; // 0 for fat32 (2880 on 1,44 MB floppys with BytesPerSec = 512)
	byte Media; // 0xF0 for floppy, 0xF8 for fixed disk
	word FATSz16; // 9 for 1,44 MB, 0 for fat32
	word SecPerTrk; // 12 for 1,44 MB floppys
	word NumHeads; // 2 for 1,44 MB floppys
	dword HiddSec; // 0 for 1,44 MB floppys
	dword TotSec32; // 0 for 1,44 MB floppys, not 0 for fat32
};

struct bpblck32
{
	dword FATSz32; // size of fat in sectors
	word ExtFlags; // 0 should work, see fat documentation!
	word FSVer; // 0
	dword RootClus; // should be 2
	word FSInfo; // should be 1
	word BkBootSec; // Should be 6
	byte Reserved[12]; // allways 0
};

struct bootsec16
{
	byte jmpBoot[3];
	byte OEMName[8]; // Should be MSWIN4.1
	struct bpblck bpb;
	byte DrvNum; // 0x00 for floppys, 0x80 for fixed disks
	byte Reserved1; // 0 (only used by windows nt)
	byte BootSig; // 0x29 for formatted disks
	dword VolID; // something random (date + time of creation combined)
	byte VolLab[11]; // can be anything, defaults to "NO NAME    "
	byte FilSysType[8]; // "FAT 12  ", "FAT 16  ", "FAT     "
};

struct bootsec32
{
	byte jmpBoot[3];
	byte OEMName[8]; // Should be MSWIN4.1
	struct bpblck bpb;
	struct bpblck32 bpb32;
	byte DrvNum; // 0x00 for floppys, 0x80 for fixed disks
	byte Reserved1; // 0 (only used by windows nt)
	byte BootSig; // 0x29 for formatted disks
	dword VolID; // something random (date + time of creation combined)
	byte VolLab[11]; // can be anything, defaults to "NO NAME    "
	byte FilSysType[8]; // "FAT 32  "
};


struct direntry
{
	byte Name[11];
	byte Attr;
	byte NTRes; // should be 0!
	byte CrtTimeTenth;
	word CrtTime;
	word CrtDate;
	word LstAccDate;
	word FstClusHI;
	word WrtTime;
	word WrtDate;
	word FstClusLO;
	dword FileSize;
	dword sec; // sector (not part of real dirent)
	dword off; // offset (")
};

void format_disk(dword vid, byte label[11], int fstype);
void disp_bs12(dword startsec);
void disp_dirent(struct bootsec16 bs, dword startsec, dword offset);
struct bootsec16 read_bs12(int startsec);
void show_dir(char *name, dword *pos);
int write_file(FILE *file, int size, char *filename);
int check_dir(const char *name, dword *pos);

#endif
