#include <datatypes.h>
#include <string.h>
#include <support.h>
#include <stdio.h>
#include <drivers/timer.h>
#include <drivers/pic.h>
#include <drivers/mem/mem.h>
#include <drivers/block/ide.h>

#include <interrupts.h>
#include <irqa.h>

#define MAX_RETRY	3
#define MAX_WAIT	30
#define WAIT_PID	3
#define WAIT_ID		30000
word Interrupt = 0;

static struct IDE_DRIVE ide_drive[8];

void ltrim(char *str)
{
	int i, j;

	if (strlen(str) == 0) return;
	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] != 32) break;
		for (j=i+1; j<strlen(str); j++)
			str[j-1] = str[j];
		str[strlen(str)] = '\0';
	}
}

void rtrim(char *str)
{
	int i;

	if (strlen(str) == 0) return;
	for (i = strlen(str) - 1; i >= 0; i--)
	{
		if (str[i] != 32) break;
		str[i] = '\0';
	}
}

char * SwapBytes(unsigned char * source, unsigned char bswap, unsigned char count)
{
	char *orig = source;
	unsigned char i, save;

	if (source)
	{
		if (bswap)
		{
			for (i=0;i<count;i++)
			{
				if (i & 1)	continue;
				save = orig[i];
				orig[i] = orig[i+1];
				orig[i+1] = save;
			}
		}
		ltrim(orig);
		rtrim(orig);
	}
	return orig;
}


char CheckLBA(ide_data disk)
{
	dword lba_sects, chs_sects;

	// The ATA spec tells large drives to return
	// C/H/S = 16383/16/63 independent of their size.
	// Some drives can be jumpered to use 15 heads instead of 16.
	// Some drives can be jumpered to use 4092 cyls instead of 16383. 
	
	if (disk.cyls == 16383 && disk.sectors == 63 &&
		(disk.heads == 15 || disk.heads == 16) &&
		(disk.lba_capacity >= (dword)(16383*16*disk.heads)))
		return 1;
	lba_sects = disk.lba_capacity;
	chs_sects = disk.cyls*disk.heads*disk.sectors;
	if (lba_sects - chs_sects < (chs_sects / 10))
		return 1;
	return 0;
}

dword CalcMB(ide_data disk)
{
	
	if (CheckLBA(disk))
		return (dword)((disk.lba_capacity*16) / 2048);
	return ((dword)disk.heads*(dword)disk.sectors*(dword)disk.cyls)/2048;
}

/* helper functions */
// Called by IRQ handler
void ide_process14(void)
{
	/* signal operation finished */
	Interrupt |= 0x4000;
}

// Called by IRQ handler
void ide_process15(void)
{
	/* signal operation finished */
	Interrupt |= 0x8000;
}

int awaitInterrupt(word IRQMask, unsigned Timeout)
{
	word Intr;
	
	for(; Timeout; Timeout--)
	{
		Intr = Interrupt & IRQMask;
		if (Intr) break;
		delay(1);
	}
	/* XXX - blocking delay - fix */
	if (Timeout == 0)
		return 0;
	Interrupt &= ~Intr;
	return Intr;
}

/*****************************************************************************	
name:	insw	
action:	reads Count words (16-bit) from I/O port Adr to		
	memory location Data
*****************************************************************************/
void insw(unsigned Adr, word *Data, unsigned Count)
{	
	for(; Count; Count--)		
        	*Data++=inportw(Adr);
}

/*****************************************************************************	
name:	outsw
action:	writes Count words (16-bit) from memory location Data
	to I/O port Adr
*****************************************************************************/
void outsw(unsigned Adr, word *Data, unsigned Count)
{
	for(; Count; Count--)
		outportw(Adr, *Data++);
}

char soft_reset(unsigned int IOAdr)
{
	unsigned int Temp;

	dprintf("ide: Port %X, soft reset...\n", IOAdr);
	outportb(IOAdr + ATA_REG_SLCT, 0x0E);
	delay(100);// release soft reset AND enable interrupts from drive
	outportb(IOAdr + ATA_REG_SLCT, 0x08);
	delay(100);// wait for master
	for(Temp=2000; Temp; Temp--)	// XXX - why 2000?
	{
		if((inportb(IOAdr + ATA_REG_STAT) & 0x80) == 0)
			break;
		delay(50);
	}
	// XXX - blocking delay
	if(Temp == 0)
	{
		dprintf("ide: no master on port %X\n", IOAdr);
		return -1;
	}
	return 0;
}

int ide_select(unsigned int IOAdr, unsigned char Sel)
{
	unsigned int Temp;
	
	Temp = inportb(IOAdr + ATA_REG_DRVHD);
	if(((Temp ^ Sel) & 0x10) == 0)
		return(0); /* already selected */
	outportb(IOAdr + ATA_REG_DRVHD, Sel);
	delay(100);
	for(Temp=MAX_WAIT; Temp; Temp--)
	{
		if((inportb(IOAdr + ATA_REG_STAT) & 0x80) == 0)
			break;
		delay(50);
	}
	/* this _must_ be polled, I guess (sigh) */
	return (Temp == 0);
}

void ide_probe(unsigned char no)
{
	unsigned int Temp, Temp1, Temp2;
	unsigned int IOAdr = ide_addr[no].port;
	unsigned char Sel = ide_addr[no].out;
	
	ide_select(IOAdr, Sel);
	Temp1 = inportb(IOAdr + ATA_REG_CNT);
	Temp2 = inportb(IOAdr + ATA_REG_SECT);
	if (Temp1 != 0x01 || Temp2 != 0x01)
	{
		dprintf("ide: nothing there\n");
		return;
	}
	Temp1 =inportb(IOAdr + ATA_REG_LOCYL);
	Temp2 =inportb(IOAdr + ATA_REG_HICYL);
	Temp = inportb(IOAdr + ATA_REG_STAT);	
	if (Temp1 == 0x14 && Temp2 == 0xEB)
	{
		dprintf("ide: ATAPI CD-ROM\n");
		ide_drive[no].flags |= ATA_FLG_ATAPI;
		// issue ATAPI 'identify drive' command
		Temp1 = ATA_CMD_PID;
		outportb(IOAdr + ATA_REG_CMD, Temp1);
		Temp = (unsigned char)WAIT_PID;
	}
	else
	if(Temp1 == 0 && Temp2 == 0 && Temp)
	{
		dprintf("ide: ATA hard drive\n");
		// issue ATA 'identify drive' command
		Temp1 = ATA_CMD_ID;
		outportb(IOAdr + ATA_REG_CMD, Temp1);
		Temp = (unsigned char)WAIT_ID;
	}
	else
	{
		dprintf("ide: unknown drive type, skipping\n");
		return;
	}
	ide_drive[no].port = IOAdr;
	ide_drive[no].out = Sel;
	delay(500);	// some delay for drive to response
	// grab info returned by 'identify'
	inportb(IOAdr + ATA_REG_STAT);
	// for ATAPI CD-ROM, you MUST read 512 bytes here, ordrive will go comatose
	insw(IOAdr + ATA_REG_DATA, (word*)&ide_drive[no].data, sizeof(ide_drive[no].data) / 2);
	Temp2 = 1;
	if (Temp1 == ATA_CMD_PID)
	// model name is not byte swapped for NEC, Mitsumi, and Pioneer drives
	{
		if ((ide_drive[no].data.model[0] == 'N' && ide_drive[no].data.model[1] == 'E') ||
			(ide_drive[no].data.model[0] == 'F' && ide_drive[no].data.model[1] == 'X') ||
			(ide_drive[no].data.model[0] == 'P' && ide_drive[no].data.model[1] == 'i'))
				Temp2=0;
	}
	ide_drive[no].model = SwapBytes(ide_drive[no].data.model, Temp2, 40);
	ide_drive[no].serial = SwapBytes(ide_drive[no].data.serial, Temp2, 20);
	ide_drive[no].rev = SwapBytes(ide_drive[no].data.rev, Temp2, 8);
	if (ide_drive[no].data.capability & 1)
		ide_drive[no].flags |= ATA_FLG_DMA;
	if(ide_drive[no].data.capability & 2)
		ide_drive[no].flags |= ATA_FLG_LBA;
	// By Dobbs, I'll figure this out yet. Linux ide.c requires
	// (DriveInfo.MultSectValid & 1) && DriveInfo.MultSect
	// The magic value then comes from DriveInfo.MaxMult
	// QUANTUM FIREBALL ST2.1A			MaxMult=16	MultSect=16	MultSectValid=1
	// Conner Peripherals 850MB - CFS850A	MaxMult=16	MultSect=0	MultSectValid=1
	// (Seagate) st3144AT			MaxMult=0	MultSect=0	MultSectValid=0
	if((ide_drive[no].data.multsect_valid & 1) && ide_drive[no].data.multsect)
		Temp = ide_drive[no].data.max_multsect;
	else
		Temp = 1;
	ide_drive[no].data.multsect = Temp;
}

int ide_init(void)
{
	int no;
	byte Temp1, Temp2;
	
	dprintf("ide: scanning ports...\n");
	for (no=0; no<8; no+=2)
	{
		outportb(ide_addr[no].port + ATA_REG_CNT, 0x55);
		outportb(ide_addr[no].port + ATA_REG_SECT, 0xAA);
		delay(500);
		Temp1 = inportb(ide_addr[no].port + ATA_REG_CNT);
		Temp2 = inportb(ide_addr[no].port + ATA_REG_SECT);
		if(Temp1 != 0x55 || Temp2 != 0xAA)
		{
			continue;
		}
		soft_reset(ide_addr[no].port);		ide_probe(no);
		soft_reset(ide_addr[no+1].port);	ide_probe(no+1);

	}
	for (no=0; no<8; no++)
	{
		if (!ide_drive[no].port) continue;
		
		dprintf("%s, ", ide_drive[no].model);
		dprintf("%s, ", ide_drive[no].serial);
		dprintf("%s, ", ide_drive[no].rev);
		if ((ide_drive[no].flags & ATA_FLG_ATAPI) == 0)
			dprintf("CHS=%d:%d:%d ", ide_drive[no].data.cyls, ide_drive[no].data.heads,
		ide_drive[no].data.sectors);
		if (ide_drive[no].flags & ATA_FLG_DMA)
			dprintf("DMA ");
		if (ide_drive[no].flags & ATA_FLG_LBA)
			dprintf("LBA ");
		dprintf("\n");
	}

	interrupt_install(INT_HARDWARE, 14, isr_ide_14);
	interrupt_install(INT_HARDWARE, 15, isr_ide_15);
//	irq_enable(14);
//	irq_enable(15);

	return 0;
}
