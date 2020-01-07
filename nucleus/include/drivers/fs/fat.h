#ifndef _FAT_H_
#define _FAT_H

#pragma pack (1)

#define _A_NORMAL   0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY   0x01    /* Read only file */
#define _A_HIDDEN   0x02    /* Hidden file */
#define _A_SYSTEM   0x04    /* System file */
#define _A_VOLID    0x08    /* Volume ID file */
#define _A_SUBDIR   0x10    /* Subdirectory */
#define _A_ARCH     0x20    /* Archive file */ 

#define NO_ERROR				 0x00
#define ERROR_FILE_NOT_FOUND	-0x01
#define ERROR_READING_FILE		-0x02
#define ERROR_NO_FREE_HANDLE	-0x10
#define ERROR_NO_HANDLE			-0x20

struct _boot_f16
{
     unsigned char  jmp[3];				/* Must be 0xEB, 0x3C, 0x90		*/
     unsigned char  sys_id[8];			/* Probably:   "MSDOS5.0"		*/
     unsigned short bytes_per_sect;		/* Sector size in bytes (512)		*/
     unsigned char  sects_per_cluster;	/* Sectors per cluster (1,2,4,...,128)	*/
     unsigned short sects_reserved;		/* Reserved sectors at the beginning	*/
     unsigned char  num_fats;			/* Number of FAT copies (1 or 2)	*/
     unsigned short root_entries;		/* Root directory entries		*/
     unsigned short sects_total;		/* Total sectors (if less 64k)		*/
     unsigned char  media_desc;			/* Media descriptor byte (F8h for HD)	*/
     unsigned short sects_per_fat;		/* Sectors per fat			*/
     unsigned short sects_per_track;	/* Sectors per track			*/
     unsigned short num_heads;			/* Sides				*/
     unsigned long  sects_hidden;		/* Special hidden sectors		*/
     unsigned long  big_total;			/* Big total number of sectors  	*/
     unsigned char  drive_num;			/* Drive number				*/
     unsigned char  reserved;
     unsigned char  signature;			/* Extended Boot Record signature (29h)	*/
     unsigned long  serial_num;			/* Volume serial number			*/
     unsigned char  label[11];			/* Volume label				*/
     unsigned char  fs_id[8];			/* File system id			*/
     unsigned char  xcode[448];			/* Loader executable code		*/
     unsigned short magic_num;			/* Magic number (Must be 0xAA55) 	*/
} __attribute__((packed));

#define boot_f16 _boot_f16

struct _dir_entry
{
	byte  filename[8];		// base name
	byte  ext[3];			// extension
	byte  attr;		// file or directory attributes
	byte  res[10];
	word  time;		// creation or last modification time
	word  date;		// creation or last modification date
	word  startcluster;	// starting cluster of the file or directory
	dword filesize;		// size of the file in bytes
};

#define dir_entry _dir_entry

struct _find_t
{
	byte attrib;
	byte inode;
	word wr_time;
	word wr_date;
	dword size;
	byte name[256];
};

#define find_t _find_t

struct _fat_file
{
	dword init;
	dword pos;
	dword inode;
};

#define fat_file _fat_file

int fs_fat_init(void);
int _fat_findfirst(char *name, unsigned int attr, struct find_t *result);
int _fat_findnext(struct _find_t *result);
int _fat_fassign(char *name, unsigned int mode);
int _fat_fread(unsigned int handle, void * buffer, unsigned int count);
int _fat_fclose(unsigned int handle);

#endif
