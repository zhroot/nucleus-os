/*
v0.1: 12.04.2004 TDS
	- Read fat
v0.2: 01.05.2004 Doug Gale
	- Fixed get_fat_entry
*/
#include <datatypes.h>
#include <support.h>
#include <stdio.h>
#include <string.h>
#include <drivers/fs/fat.h>
#include <drivers/mem/mem.h>
#include <drivers/block/floppy.h>

#define ATTR_READONLY	0x01 /* read-only */
#define ATTR_HIDDEN		0x02 /* hidden */
#define ATTR_SYS		0x04 /* system */
#define ATTR_VOLUME		0x08 /* volume label */
#define ATTR_DIR		0x10 /* directory */
#define ATTR_ARCHIVED	0x20 /* archived */ 

#define FAT12  0
#define FAT16  1
#define FAT32  2

const char * BAD_CHARS = ".\"\\\[]:;|=,";
struct boot_f16 bpb;
dword root_start, fat_start, data_start;
word clusters;
byte * fat_buf;
int fat_type;

char month[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

dword fat_end(void)
{
	switch(fat_type)
	{
		case FAT12: return 0x00000FF8;
		case FAT16: return 0x0000FFF8;
		case FAT32: return 0xFFFFFFF8;
 	}
 	return 0x00;
}

struct dir_entry * root;

dword get_fat_entry(dword start, byte type)
{
	dword usl;

	switch (type)
	{		
	case FAT12:
		usl = (start >> 1) + start;
		if (start & 1)  // if odd, get high 12 bits
			return (*((unsigned short *)(fat_buf+usl))) >> 4;
		// if even, get low 12 bits
		return (*((unsigned short *)(fat_buf+usl))) & 0x0FFF;
	case FAT16:
		return *((unsigned short*)fat_buf + start);
	case FAT32:
		return (*((dword *)(fat_buf+(start<<2))));
	default:
		return 0x00;
	}
}

// WILDCARD SEARCH
//    Returns 1-> Match  |  0-> No Match
// Limitation:     IT IS CASE-SENSITIVE

int wildcmp(char *wild, char *string)
{
	char *cp = "", *mp = "";
	
	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
		{
			return 0;
		}
		wild++;
		string++;
	}
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = string + 1;
		}
		else
		if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
	while (*wild == '*')
	{
		wild++;
	}
	return !*wild;
}

// change me :-)
// how do we search with a handle?
char pattern[256], last[256];

int _fat_findfirst(char *name, unsigned int attr, struct find_t *result)
{
	word i, j;
	char *f;
	char filename[32];
	
	strncpy(pattern, name, sizeof(pattern)-1);
	pattern[sizeof(pattern)-1] = 0;

	for (i=0; i<bpb.root_entries; i++)
	{
		// if root[i].name[0] == '\0', we can quit looking per DOS 2.x's specs of a FAT
		if (!root[i].filename[0])
			break;

		if ((unsigned char) root[i].filename[0] != 0xE5)
		{
			f = filename;
			for (j=0; j<8; j++) {
				*f = root[i].filename[j];
				if (*f == ' ') break;
				f++;
			}
			*f++ = '.';
			for (j=0; j<3; j++) {
				*f = root[i].ext[j];
				if (*f == ' ') break;
				f++;
			}
			*f = 0x00;			
			
			if (root[i].attr == 0x0F)
			{
				printf("LFN slot entry");
			}
			else
			if (wildcmp(name, filename))
			{
				strcpy(result->name, filename);
				strcpy(last, filename);
				result->size = root[i].filesize;
				result->wr_time = root[i].time;
				result->wr_date = root[i].date;
				result->inode = i;
				
				return 0;
			}
		}
	}
	return 0xFF;
}

int _fat_findnext(struct _find_t *result)
{
	word i, j;
	char *f;
	char filename[32];
	BOOL next = FALSE;
	
	for (i=0; i<bpb.root_entries; i++)
	{
		// if root[i].name[0] == '\0', we can quit looking per DOS 2.x's specs of a FAT
		if (root[i].filename[0] && ((unsigned char) root[i].filename[0] != 0xE5)) 
		{
			f = filename;
			for (j=0; j<8; j++) {
				*f = root[i].filename[j];
				if (*f == ' ') break;
				f++;
			}
			*f++ = '.';
			for (j=0; j<3; j++) {
				*f = root[i].ext[j];
				if (*f == ' ') break;
				f++;
			}
			*f = 0x00;			
			
			if (root[i].attr == 0x0F)
			{
				printf("LFN slot entry");
			}
			else if (!strcmp(filename, last))
			{
				next = TRUE;
			}
			else if (next == TRUE && wildcmp(pattern, filename))
			{
				strcpy(result->name, filename);
				strcpy(last, filename);
				result->size = root[i].filesize;
				result->wr_time = root[i].time;
				result->wr_date = root[i].date;
				result->inode = i;
				return 0;
			}
		}
	}
	return 0xFF;
}

#define MAX_OPEN_FILES		(10+1)	// Add 1 because first entry is unused

static struct _fat_file _open_files[MAX_OPEN_FILES];

int _fat_fassign(char *name, unsigned int mode)
{
	struct find_t tmp;
	int i, handle;

	if (_fat_findfirst(name, 0xFF, &tmp) != 0x00)
		return ERROR_FILE_NOT_FOUND;

	handle = ERROR_NO_FREE_HANDLE;	
	for (i=0; i<MAX_OPEN_FILES; i++)
	{
		if (_open_files[i].init == 0x00)
		{
			_open_files[i].init = 0x01;
			_open_files[i].inode = tmp.inode;
			_open_files[i].pos = get_fat_entry(root[tmp.inode].startcluster, fat_type);
			handle = i;
			break;
		}
	}
	
	return handle;		
}

// TODO: check for connected sectors and read them once
// Returns number of sectors read.
// Returns zero at end-of-file.
// If you read zero sectors then it will return zero even if not at EOF.
// Negative return values are errors
int _fat_fread(unsigned int handle, void * buffer, unsigned int count)
{
	dword sector, readcount;

	if (handle > MAX_OPEN_FILES)
		panic();
	
	if (!handle)	return ERROR_NO_HANDLE;	

	readcount = 0;
	while (_open_files[handle].pos < fat_end() && count--) {
		sector = (_open_files[handle].pos - 2) + (dword)data_start;
		if (!floppy_ioctl(0, FLOPPY_READ, buffer, sector, 1))
			return ERROR_READING_FILE;
		readcount++;
		buffer += 512;
		_open_files[handle].pos = get_fat_entry(_open_files[handle].pos, fat_type);
	}
	
	// DEBUG
	printf("pos: %ld\n", _open_files[handle].pos);

	return readcount;
}

int _fat_fclose(unsigned int handle)
{
	if (!handle)	return ERROR_NO_HANDLE;
	
	_open_files[handle].init = 0x00;
	return NO_ERROR;
}

int fs_fat_init(void)
{
	word root_size, j;

	// Mark first entry as permanently taken, to avoid zero filehandle
	_open_files[0].init = 0x01;

	if (floppy_ioctl(0, FLOPPY_READ, (char *)&bpb, 0, 1))
	{	
		fat_start = bpb.sects_reserved;
		root_start = fat_start + (bpb.num_fats * bpb.sects_per_fat);
		data_start = root_start + (bpb.root_entries * sizeof(struct dir_entry) / bpb.bytes_per_sect);
		clusters = (bpb.sects_total - root_start) / bpb.sects_per_cluster;
		if (clusters < 4085)
		{
			fat_type = FAT12;
			printf("fs: fat12 detected...\n");
		}
		else
		if (clusters < 65525)
		{
			fat_type = FAT16;
			printf("fs: fat16 detected...\n");
		}
		else
		{
			fat_type = FAT32;
			printf("fs: fat32 detected...\n");
		}
			fat_buf = malloc(bpb.sects_per_fat * bpb.bytes_per_sect);
			printf("fs: reading FAT: %d bytes (%d sectors)...", bpb.sects_per_fat * bpb.bytes_per_sect, bpb.sects_per_fat);
			if (floppy_ioctl(0, FLOPPY_READ, (byte *)fat_buf, fat_start, bpb.sects_per_fat))
			{
				printf("OK\n");
				root_size = bpb.root_entries * sizeof(struct dir_entry);
				j = bpb.root_entries * sizeof(struct dir_entry);
				j = (j % bpb.bytes_per_sect) ? ((j / bpb.bytes_per_sect) + 1) : (j / bpb.bytes_per_sect);
				root = malloc(root_size);
				printf("fs: fat12: reading ROOT entries %d bytes (%d sectors)...", root_size, j);
				if (floppy_ioctl(0, FLOPPY_READ, (char *)root, root_start, j))
				{
					printf("OK\n");					
				}
//				free(root);
			}
			else
				printf("Error reading FAT12...\n");
//			free(fat_buf);
	}
	else
		printf("fs: fat: read error...\n");
	return 0x00;
}

