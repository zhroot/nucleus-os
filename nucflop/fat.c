#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fat.h"

#define RDIMG8(A) (image[startsec * bs.bpb.BytesPerSec + A])

#define RDIMG16(A) (image[startsec * bs.bpb.BytesPerSec + A] | \
				   (image[startsec * bs.bpb.BytesPerSec + A + 1] << 8))

#define RDIMG32(A) (image[startsec * bs.bpb.BytesPerSec + A] | \
				   (image[startsec * bs.bpb.BytesPerSec + A + 1] << 8) | \
				   ((image[startsec * bs.bpb.BytesPerSec + A + 2] << 8) << 8) | \
                   (image[startsec * bs.bpb.BytesPerSec + A + 3] << 10))


#define WRIMG8(A, B) image[startsec * bs.bpb.BytesPerSec + A] = B

#define WRIMG16(A, B) image[startsec * bs.bpb.BytesPerSec + A] = (B & 0xFF); \
					  image[startsec * bs.bpb.BytesPerSec + A + 1] = (B >> 8)

#define WRIMG32(A, B) image[startsec * bs.bpb.BytesPerSec + A] = (byte)(B & 0xFF); \
					  image[startsec * bs.bpb.BytesPerSec + A + 1] = (byte)((B >> 8) & 0xFF); \
					  image[startsec * bs.bpb.BytesPerSec + A + 2] = (byte)(((B >> 8) >> 8) & 0xFF); \
					  image[startsec * bs.bpb.BytesPerSec + A + 3] = (byte)(((B & 0xFF000000) >> 10) & 0xFF)


extern unsigned char *image;

void write_bs12(struct bootsec16 bs, int startsec)
{
	int i;

	for(i = 0; i < 3; i++)WRIMG8(i, bs.jmpBoot[i]);
	for(i = 0; i < 8; i++)WRIMG8(i + 3, bs.OEMName[i]);
	WRIMG16(11, bs.bpb.BytesPerSec);
	WRIMG8(13, bs.bpb.SecPerCluster);
	WRIMG16(14, bs.bpb.RsvdSecCnt);
	WRIMG8(16, bs.bpb.NumFATs);
	WRIMG16(17, bs.bpb.RootEntCnt);
	WRIMG16(19, bs.bpb.TotSec16);
	WRIMG8(21, bs.bpb.Media);
	WRIMG16(22, bs.bpb.FATSz16);
	WRIMG16(24, bs.bpb.SecPerTrk);
	WRIMG16(26, bs.bpb.NumHeads);
	WRIMG32(28, bs.bpb.HiddSec);
	WRIMG32(32, bs.bpb.TotSec32);
	WRIMG8(36, bs.DrvNum);
	WRIMG8(37, bs.Reserved1);
	WRIMG8(38, bs.BootSig);
	WRIMG32(39, bs.VolID);
	for(i = 0; i < 11; i++)WRIMG8(43 + i, bs.VolLab[i]);
	for(i = 0; i <  8; i++)WRIMG8(54 + i, bs.FilSysType[i]);
}

struct bootsec16 read_bs12(int startsec)
{
	int i;
	struct bootsec16 bs;

	for(i = 0; i < 3; i++)bs.jmpBoot[i] = RDIMG8(i    );
	for(i = 0; i < 8; i++)bs.OEMName[i] = RDIMG8(i + 3);
	bs.bpb.BytesPerSec = RDIMG16(11);
	bs.bpb.SecPerCluster = RDIMG8(13);
	bs.bpb.RsvdSecCnt = RDIMG16(14);
	bs.bpb.NumFATs = RDIMG8(16);
	bs.bpb.RootEntCnt = RDIMG16(17);
	bs.bpb.TotSec16 = RDIMG16(19);
	bs.bpb.Media = RDIMG8(21);
	bs.bpb.FATSz16 = RDIMG16(22);
	bs.bpb.SecPerTrk = RDIMG16(24);
	bs.bpb.NumHeads = RDIMG16(26);
	bs.bpb.HiddSec = RDIMG32(28);
	bs.bpb.TotSec32 = RDIMG32(32);
	bs.DrvNum = RDIMG8(36);
	bs.Reserved1 = RDIMG8(37);
	bs.BootSig = RDIMG8(38);
	bs.VolID = RDIMG32(39);
	for(i = 0; i < 11; i++)bs.VolLab[i] = RDIMG8(43 + i);
	for(i = 0; i <  8; i++)bs.FilSysType[i] = RDIMG8(54 + i);

	return bs;
}

void disp_bs12(dword startsec)
{
	int i;
	struct bootsec16 bs;

	bs = read_bs12(startsec);
	printf("jmpBoot = ");
	for(i = 0; i < 3; i++)printf("0x%02X ", bs.jmpBoot[i]);
	printf("\nOEMName = \"");
	for(i = 0; i < 8; i++)printf("%c", bs.OEMName[i]);
	printf("\"\nBytesPerSec = %d\n", bs.bpb.BytesPerSec);
	printf("SecPerCluster = %d\n", bs.bpb.SecPerCluster);
	printf("RsvdSecCnt = %d\n", bs.bpb.RsvdSecCnt);
	printf("NumFats = %d\n", bs.bpb.NumFATs);
	printf("RootEntCnt = 0x%02X\n", bs.bpb.RootEntCnt);
	printf("TotSec16 = %d\n", bs.bpb.TotSec16);
	printf("Media = %02X\n", bs.bpb.Media);
	printf("FATSz16 = %d\n", bs.bpb.FATSz16);
	printf("SecPerTrk = %d\n", bs.bpb.SecPerTrk);
	printf("NumHeads = %d\n", bs.bpb.NumHeads);
	printf("HiddSec = %08lX\n", bs.bpb.HiddSec);
	printf("TotSec32 = %08lX\n", bs.bpb.TotSec32);
	printf("DrvNum = %d\n", bs.DrvNum);
	printf("Reserved1 = %d\n", bs.Reserved1);
	printf("BootSig = 0x%02X\n", bs.BootSig);
	printf("VolID = %08lX\n", bs.VolID);
	printf("VolLab = \"");
	for(i = 0; i < 11; i++)printf("%c", bs.VolLab[i]);
	printf("\"\nFilSysType = \"");
	for(i = 0; i <  8; i++)printf("%c", bs.FilSysType[i]);
	printf("\"\n");
}

void create_bs12(dword vid, char *label)
{
	struct bootsec16 bs;

	bs.jmpBoot[0] = 0xEB;
	bs.jmpBoot[1] = 0x90;
	bs.jmpBoot[2] = 0x90;
	bs.OEMName[0] = 'M';
	bs.OEMName[1] = 'S';
	bs.OEMName[2] = 'W';
	bs.OEMName[3] = 'I';
	bs.OEMName[4] = 'N';
	bs.OEMName[5] = '4';
	bs.OEMName[6] = '.';
	bs.OEMName[7] = '1';
	bs.bpb.BytesPerSec = 512;
	bs.bpb.SecPerCluster = 1;
	bs.bpb.RsvdSecCnt = 1;
	bs.bpb.NumFATs = 2;
	bs.bpb.RootEntCnt = 0xE0;
	bs.bpb.TotSec16 = 2880;
	bs.bpb.Media = 0xF0;
	bs.bpb.FATSz16 = 9;
	bs.bpb.SecPerTrk = 18;
	bs.bpb.NumHeads = 2;
	bs.bpb.HiddSec = 0;
	bs.bpb.TotSec32 = 0;
	bs.DrvNum = 0;
	bs.Reserved1 = 0;
	bs.BootSig = 0x29;
	bs.VolID = 0x19110323;
	bs.VolLab[0] = label[0];
	bs.VolLab[1] = label[1];
	bs.VolLab[2] = label[2];
	bs.VolLab[3] = label[3];
	bs.VolLab[4] = label[4];
	bs.VolLab[5] = label[5];
	bs.VolLab[6] = label[6];
	bs.VolLab[7] = label[7];
	bs.VolLab[8] = label[8];
	bs.VolLab[9] = label[9];
	bs.VolLab[10] = label[10];
	bs.FilSysType[0] = 'F';
	bs.FilSysType[1] = 'A';
	bs.FilSysType[2] = 'T';
	bs.FilSysType[3] = '1';
	bs.FilSysType[4] = '2';
	bs.FilSysType[5] = ' ';
	bs.FilSysType[6] = ' ';
	bs.FilSysType[7] = ' ';

	write_bs12(bs, 0);
}

/*dword FirstSectorofCluster(struct bootsec16 bs, int Cluster)
{
	return bs.bpb.RsvdSecCnt + (bs.bpb.NumFATs * bs.bpb.FATSz16) + RootDirSectors;
}*/

dword FirstSectorOfCluster(struct bootsec16 bs, dword cluster)
{
	dword RootDirSectors, FirstDataSector;

	RootDirSectors = ((bs.bpb.RootEntCnt * 32) + (bs.bpb.BytesPerSec - 1)) / bs.bpb.BytesPerSec;
	if(((bs.bpb.RootEntCnt * 32) + (bs.bpb.BytesPerSec - 1)) % bs.bpb.BytesPerSec)RootDirSectors++;

	FirstDataSector = bs.bpb.RsvdSecCnt + (bs.bpb.NumFATs * bs.bpb.FATSz16) + RootDirSectors;

	return FirstDataSector + ((cluster - 3) * bs.bpb.SecPerCluster);
}


void create_root12(void)
{
	struct bootsec16 bs;
	int RootDirSectors, FirstDataSector/*, FirstSectorofCluster*/, i;
	
	bs = read_bs12(0);

	RootDirSectors = ((bs.bpb.RootEntCnt * 32) + (bs.bpb.BytesPerSec - 1)) / bs.bpb.BytesPerSec;
	if(((bs.bpb.RootEntCnt * 32) + (bs.bpb.BytesPerSec - 1)) % bs.bpb.BytesPerSec)RootDirSectors++;

	FirstDataSector = bs.bpb.RsvdSecCnt + (bs.bpb.NumFATs * bs.bpb.FATSz16) + RootDirSectors;
	
	// the most important thing is, that the floppy is recognized by BIOS
	image[0x1FE] = 0x55;
	image[0x1FF] = 0xAA;

	// now we write the correct values for the reserved 2 cluster of FAT
	for(i = 0; i < bs.bpb.NumFATs; i++)
	{
		image[(bs.bpb.BytesPerSec * (i * bs.bpb.FATSz16 + 1))    ] = bs.bpb.Media;
		image[(bs.bpb.BytesPerSec * (i * bs.bpb.FATSz16 + 1)) + 1] = 0xFF;
		image[(bs.bpb.BytesPerSec * (i * bs.bpb.FATSz16 + 1)) + 2] = 0xFF;
	}

	// ok, now lets write the root entry

}

struct direntry read_dirent(struct bootsec16 bs, dword startsec, dword offset)
{
	int i;
	struct direntry de;

	for(i = 0; i < 11; i++)de.Name[i] = RDIMG8(i + offset);
	de.Attr = RDIMG8(11 + offset);
	de.NTRes = RDIMG8(12 + offset);
	de.CrtTimeTenth = RDIMG8(13 + offset);
	de.CrtTime = RDIMG16(14 + offset);
	de.CrtDate = RDIMG16(16 + offset);
	de.LstAccDate = RDIMG16(18 + offset);
	de.FstClusHI = RDIMG16(20 + offset);
	de.WrtTime = RDIMG16(22 + offset);
	de.WrtDate = RDIMG16(24 + offset);
	de.FstClusLO = RDIMG16(26 + offset);
	de.FileSize = RDIMG32(28 + offset);

	return de;
}

void write_dirent(struct bootsec16 bs, struct direntry de)
{
	int i, startsec = de.sec;

	for(i = 0; i < 11; i++)WRIMG8(de.off + i, de.Name[i]);
	WRIMG8(de.off + 11, de.Attr);
	WRIMG8(de.off + 12, de.NTRes);
	WRIMG8(de.off + 13, de.CrtTimeTenth);
	WRIMG16(de.off + 14, de.CrtTime);
	WRIMG16(de.off + 16, de.CrtDate);
	WRIMG16(de.off + 18, de.LstAccDate);
	WRIMG16(de.off + 20, de.FstClusHI);
	WRIMG16(de.off + 22, de.WrtTime);
	WRIMG16(de.off + 24, de.WrtDate);
	WRIMG16(de.off + 26, de.FstClusLO);
	WRIMG32(de.off + 28, de.FileSize);
}

void disp_dirent(struct bootsec16 bs, dword startsec, dword offset)
{
	int i;

	struct direntry de;

	de = read_dirent(bs, startsec, offset);

	printf("Name = ");
	for(i = 0; i < 11; i++)printf("%c", de.Name[i]);
	printf("\nAttr = 0x%02X\n", de.Attr);
	printf("NTRes = 0x%02X\n", de.NTRes);
	printf("CrtTimeTenth = 0x%02X\n", de.CrtTimeTenth);
	printf("CrtTime = 0x%04X\n", de.CrtTime);
	printf("CrtDate = 0x%04X\n", de.CrtDate);
	printf("LstAccDate = 0x%04X\n", de.LstAccDate);
	printf("FstClusHI = 0x%04X\n", de.FstClusHI);
	printf("WrtTime = 0x%04X\n", de.WrtTime);
	printf("WrtDate = 0x%04X\n", de.WrtDate);
	printf("FstClusLO = 0x%04X\n", de.FstClusLO);
	printf("FileSize = 0x%08lX\n", (dword)de.FileSize);
}

void format_disk(dword vid, byte label[11], int fstype)
{
	byte label1[11] = "           ";
	strcpy(label1, label);
	strupr(label1);
	create_bs12(vid, label1);
	create_root12();
}

dword seek_dirent(struct bootsec16 bs, const char name[12], dword sector)
{
	struct direntry de;
	int i = 0, j;
	char dir[12];

	de = read_dirent(bs, sector, i);
	
	while(de.Name[0])
	{
		if(de.Name[0] != 0xE5)
		{
			if(de.Attr & ATTR_LONG_NAME)
			{
				// long filenames not supported yet
			}
			
			else
			{
				for(j = 0; j < 11; j++)
				{
					dir[j] = de.Name[j];
				}
				dir[11] = '\0';
	
				if(!strcmp(name, dir))break;
			}
		}

		if(i == 0xE0)
		{
			printf("dir not found, we would need to fetch the next cluster here, but thats not working yet!\n");
			return 0;
		}
		else i += 0x20;
		
		de = read_dirent(bs, sector, i);
	}
	
	if(i == 0)return 0;

	return FirstSectorOfCluster(bs, de.FstClusLO | ((de.FstClusHI << 8) << 8));
}

void lookup_dir(struct bootsec16 bs, const char *name, dword *dir_ptr)
{
	dword sector;
	dword i, j, k;
	char subname[12];

	// first lets compute the sector of the root dir
	sector = bs.bpb.RsvdSecCnt + bs.bpb.NumFATs * bs.bpb.FATSz16;

	
	// is the dir, we are searching for, actually root?
	if(!strlen(name))
	{
		*dir_ptr = sector;
		return;
	}

	// do we have to deal with "."?
	if((name[strlen(name) - 1] == '.') && (name[strlen(name) - 2] == '/'))
	{
		return;
	}

	// is dir ".." and actually root?
	if((name[strlen(name) - 1] == '.') && (name[strlen(name) - 2] == '.'))
	{
		// what the hell does this code do HEEEEEEEEEEEELPPPPPPPP!!!!!!!!!
		//if(bs, "..", *dir_ptr);
		return;
	}

	// okay, now we need a loop to parse the name string
	for(i = 0; i < strlen(name); i++)
	{
		for(j = i; j < strlen(name); j++)
		{
			if(name[j] == '/') break;
		}

		for(k = i; k < j; k++)subname[k - i] = toupper(name[k]);
		// padding subname with zeros
		for(; k - i < 11; k++)subname[k - i] = (char)0x20;
		subname[11] = '\0';

		i = j;

		sector = seek_dirent(bs, subname, sector);
		
		if(!sector)
		{
			*dir_ptr = 0;
			return;
		}
	}
	
	*dir_ptr = sector;
}

void show_dir(char *name, dword *pos)
{
	int i, j;
	struct direntry de;
	struct bootsec16 bs = read_bs12(0);

	if(*pos == 0)
	{
		lookup_dir(bs, name, pos);
		if(*pos == 0)
		{
			printf("directory not found!\n");
			return;
		}
	}

	printf("listing all entries of /%s\n\n", strupr(name));

	i = 0;
	
	de = read_dirent(bs, *pos, i);

	while(de.Name[0] != '\0')
	{
		if(de.Name[0] != 0xE5)
		{
			if(de.Attr & ATTR_LONG_NAME)
			{
				// handle long filenames here
			}
			else
			{
				for(j = 0; j < 11; j++)
				{
					if(j == 8)printf(" ");
					/*if(de.Name[j] != 0x20)*/printf("%c", de.Name[j]);
				}
				printf("\n");
			}
		}

		if(i == 0xE0)
		{
			printf("error: cluster seeking not finished\n");
			return;
		}
		else 
		{
			i += 0x20;
			de = read_dirent(bs, *pos, i);
		}
	}
}

int check_dir(const char *name, dword *pos)
{
	dword i;
	//struct direntry de;
	struct bootsec16 bs = read_bs12(0);

	i = *pos;
	lookup_dir(bs, name, &i);
	if(i)
	{
		*pos = i;
		return 0;
	}
	else return 1;
}

int get_disk_free()
{
	int i, startsec = 1, diskfree = 0;
	word fatentry;
	struct bootsec16 bs = read_bs12(0);

	for(i = 2; i < 2849; i++)
	{
		fatentry = RDIMG16(((i * 3) / 2));

		if(i % 2) // ungrade
		{
			if(!(fatentry >> 4)) diskfree++;
		}
		else // grade
		{
			if(!(fatentry & 0xFFF)) diskfree++;
		}
	}

	return diskfree;
}

int get_free_cluster()
{
	int i, startsec = 1;
	word fatentry;
	struct bootsec16 bs = read_bs12(0);
	
	for(i = 2; i < 2849; i++)
	{
		fatentry = RDIMG16(((i * 3) / 2));

		if(i % 2) // ungrade
		{
			if(!(fatentry >> 4)) return i;
		}
		else // grade
		{
			if(!(fatentry & 0xFFF)) return i;
		}
	}

	return -1;
}

int write_cluster(int cluster, byte *buffer)
{
	int i, startsec = cluster + 0x1F;
	struct bootsec16 bs = read_bs12(0);

	for(i = 0; i < 512; i++)
	{
		WRIMG8(i, buffer[i]);
	}

	return 0;
}

int write_fatentry(int cluster, int newentry)
{
	int startsec = 1;
	word fatentry;
	struct bootsec16 bs = read_bs12(0);

	fatentry = RDIMG16(((cluster * 3) / 2));

	if(cluster % 2) // ungrade
	{
		fatentry = (newentry << 4) + (fatentry & 0xF);
	}
	else // grade
	{
		fatentry = (newentry & 0xFFF) + (fatentry & 0xF000);
	}

	WRIMG16(((cluster * 3) / 2), fatentry);
	startsec = 10;
	WRIMG16(((cluster * 3) / 2), fatentry);
	
	return 0;
}

struct direntry find_dirlimit()
{
	struct direntry de;
	struct bootsec16 bs = read_bs12(0);

	int i, ii;

	for(i = 19; i < 33; i++)
	{
		for(ii = 0; ii < 256; ii = ii + 32)
		{
			de = read_dirent(bs, i, ii);
			if(de.Name[0] == 0x00)break;
		}
		if(de.Name[0] == 0x00)break;
	}

	de.off = ii;
	de.sec = i;
	de.Attr = 0;
	de.CrtDate = 0;
	de.CrtTime = 0;
	de.CrtTimeTenth = 0;
	de.FileSize = 0;
	de.FstClusHI = 0;
	de.FstClusLO = 0;
	de.LstAccDate = 0;
	for(i = 0; i < 11; i++)de.Name[i] = 0; 
	de.NTRes = 0;
	de.WrtDate = 0;
	de.WrtTime = 0;

	return de;
}

int write_file(FILE *file, int size, char *filename)
{
	int oldcluster, cluster, i;
	byte buffer[512];
	struct bootsec16 bs = read_bs12(0);
	struct direntry de;
	
	if((get_disk_free() * 512) < size + 512) return 1; // speicherplatz prüfen

	cluster = get_free_cluster(); // freien cluster suchen
	if(cluster == -1) return 2;

	de = find_dirlimit();

	de.FstClusLO = cluster;

	fread(buffer, 512, 1, file); // einen sektor aus der datei lesen

	for(;;)
	{
		write_cluster(cluster, buffer); // den sektor auf das image schreiben

		write_fatentry(cluster, 0xFFF); // dateiendmarke 

		if(!fread(buffer, 512, 1, file)) // einen sektor aus der datei lesen

		break;

		oldcluster = cluster; // alten cluster sichern
			
		cluster = get_free_cluster(); // freien cluster suchen
		if(cluster == -1) return 2;

		write_fatentry(oldcluster, cluster); // fat eintrag schreiben

	}

	de.Attr = 0;
	de.CrtDate = 0;
	de.CrtTime = 0;
	de.CrtTimeTenth = 0;
	de.FileSize = size;
	de.FstClusHI = 0;
	de.LstAccDate = 0;
	de.NTRes = 0;
	de.WrtDate = 0;
	de.WrtTime = 0;

	for(i = 0; i < 11; i++)de.Name[i] = toupper(filename[i]); 
	
	write_dirent(bs, de);

	return 0;	
}
