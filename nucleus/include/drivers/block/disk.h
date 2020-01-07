#ifndef _DISK_H_
#define _DISK_H_

#include <datatypes.h>

struct CHS
{
	dword cyls, heads, sectors;
};

struct disk_info
{
	unsigned long size;
	struct CHS chs;
	char description[100];
	int (*read)(const unsigned int sector, void *buffer, const dword bufsize);
	int (*write)(const unsigned int sector, const void *buffer, const dword bufsize);
};

#endif
