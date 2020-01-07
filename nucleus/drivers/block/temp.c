/*
Fixes/changes:	- CD-ROM now works properly after ATAPI 'identify'.
Bugs/to do:	- Write a new ataXfer() that
* allows reads to start in the middle of an ATA or ATAPI sector
(no odd writes until a buffer cache system is added)
(change Blk field of drivecmd [sectors] to Offset [bytes])
* allows odd-length Count in drivecmd
	- Buffer cache
	- Queued drive operations
	- TEST drives on 2nd interface (0x170, IRQ15).
	- Still some arbitrary timeout values.
	- atapiCmd2() reads only 2048 bytes at a time. Is this normal?
	- False detection of slave ATA drive with Seagate ST3144A master
	(it's jumpered properly, and Linux and the BIOS work OK).
	- Lots of blocking delays. This code is only partially interrupt-	  
	driven, and not yet suitable for a high-performance OS.
	- Code needs more error checking and error-recovery.
*****************************************************************************/
#include	<stdlib.h>	/* srand() rand() */
#include	<conio.h>	/* getch() */
#include	<stdio.h>	/* NULL printf() cprintf() */

//#define DEBUG

#ifdef DEBUG
/* extra debugging messages */
#define	PRINT_DEBUG(X)	X
#else
/* no debug msgs */
#define	PRINT_DEBUG(X)
#endif/*
////////////////////////////////////////////////////////////////////////////
LOW LEVEL FUNCTIONS AND BASIC DEFINTIONS
////////////////////////////////////////////////////////////////////////////
*/
/* delay() inp() inpw() outp() outpw() */
#include	<dos.h>
typedef unsigned char	u8;	/* 8 bits */
typedef unsigned short	u16;	/* 16 bits */
typedef unsigned long	u32;	/* 32 bits */
#define		msleep		delay
#define min(a,b)    (((a) < (b)) ? (a) : (b))

int InterruptOccured;

/*****************************************************************************	
name:	nsleep	
action:	precision delay for Count nanoseconds		
	(not yet implemented)
*****************************************************************************/
void nsleep(unsigned Count)
{
	delay(0);
}

/*****************************************************************************	
name:	insw	
action:	reads Count words (16-bit) from I/O port Adr to		
	memory location Data
*****************************************************************************/
void insw(unsigned Adr, u16 *Data, unsigned Count)
{	
	for(; Count; Count--)		
        	*Data++=inpw(Adr);
}

/*****************************************************************************	
name:	outsw
action:	writes Count words (16-bit) from memory location Data
	to I/O port Adr
*****************************************************************************/
void outsw(unsigned Adr, u16 *Data, unsigned Count)
{
	for(; Count; Count--)
		outpw(Adr, *Data++);
}

/*****************************************************************************
name:	dump
action:	hexadecimal memory dump of Count bytes at Data
*****************************************************************************/
#define		BPERL		16
/* byte/line for dump */
void dump(u8 *Data, unsigned Count)
{
	unsigned Byte1, Byte2;
	while(Count)
	{
		for(Byte1=0; Byte1 < BPERL; Byte1++, Count--)
		{
			if(Count == 0) break;
			printf("%02X ", Data[Byte1]);
		}
		printf("\t");
		for(Byte2=0; Byte2 < Byte1; Byte2++)
			printf("%c", Data[Byte2] < ' ' ? '.' : Data[Byte2]);
		printf("\n");
		Data += BPERL;
	}
}

/*****************************************************************************
name:	awaitInterrupt
action:	waits with Timeout (mS) until interrupt(s) given by bitmask
	IRQMask occur
returns:nonzero Mask value if interrupt occured, zero if timeout
*****************************************************************************/
int awaitInterrupt(u16 IRQMask, unsigned Timeout)
{
	u16 Intr;
	for(; Timeout; Timeout--)
	{
		Intr=InterruptOccured & IRQMask;
		if(Intr) break;
		msleep(1);
	}
	/* XXX - blocking delay - fix */
	if(Timeout == 0) return(0);
	InterruptOccured &= ~Intr;
	return(Intr);
}
/*
////////////////////////////////////////////////////////////////////////////
ATA/ATAPI STUFF
////////////////////////////////////////////////////////////////////////////
*/
/* Delays from Linux ide.h (milliseconds) */
#define	WAIT_READY	30	/* RDY asserted, use 5000 for notebook/APM */
#define	WAIT_ID		30000	/* ATA device responds to 'identify' */
#define	WAIT_PID	3	/* ATAPI device responds to 'identify' */
#define	WAIT_CMD	10000	/* IRQ occurs in response to command */
#define	WAIT_DRQ	20	/* DRQ asserted after ATA_CMD_WR(MUL) */
/* 'Cmd' field of 'drivecmd' structure */
#define	DRV_CMD_RD	1
#define	DRV_CMD_WR	2
/* ATA or ATAPI command structure */
typedef struct{
	u32 Blk;	/* in SECTORS */
	u32 Count;	/* in BYTES */
	u8 Dev, Cmd, *Data;
 } drivecmd;
/* ATA sector size */
#define	ATA_LG_SECTSIZE		9				/* 512 byte ATA drive sectors */
#define	ATA_SECTSIZE		(1 << (ATA_LG_SECTSIZE))	/* ATAPI sector size */
#define	ATAPI_LG_SECTSIZE	11				/* 2K CD-ROM sectors */
#define	ATAPI_SECTSIZE		(1 << (ATAPI_LG_SECTSIZE))
/* ATA drive command bytes */
#define	ATA_CMD_RD	0x20					/* read one sector */
#define	ATA_CMD_WR	0x30					/* write one sector */
#define	ATA_CMD_PKT	0xA0					/* ATAPI packet cmd */
#define	ATA_CMD_PID	0xA1					/* ATAPI identify */
#define	ATA_CMD_RDMUL	0xC4					/* read multiple sectors */
#define	ATA_CMD_WRMUL	0xC5					/* write multiple sectors */
#define	ATA_CMD_ID	0xEC					/* ATA identify */
/* ATA drive flags */
#define	ATA_FLG_ATAPI	0x0001					/* ATAPI drive */
#define ATA_FLG_LBA	0x0002					/* LBA-capable */
#define ATA_FLG_DMA	0x0004					/* DMA-capable */
/* ATA/ATAPI drive register file */
#define	ATA_REG_DATA	0					/* data (16-bit) */
#define	ATA_REG_FEAT	1					/* write: feature reg */
#define	ATA_REG_ERR	ATA_REG_FEAT				/* read: error */
#define	ATA_REG_CNT	2					/* ATA: sector count */
#define	ATA_REG_REASON	ATA_REG_CNT				/* ATAPI: interrupt reason */
#define	ATA_REG_SECT	3					/* sector */
#define	ATA_REG_LOCYL	4					/* ATA: LSB of cylinder */
#define	ATA_REG_LOCNT	ATA_REG_LOCYL				/* ATAPI: LSB of transfer count */
#define	ATA_REG_HICYL	5					/* ATA: MSB of cylinder */
#define	ATA_REG_HICNT	ATA_REG_HICYL				/* ATAPI: MSB of transfer count */
#define	ATA_REG_DRVHD	6					/* drive select; head */
#define	ATA_REG_CMD	7					/* write: drive command */
#define	ATA_REG_STAT	7					/* read: status and error flags */
#define	ATA_REG_SLCT	0x206					/* write: device control */
#define	ATA_REG_ALTST	0x206					/* read: alternate status/error */
typedef struct /* 'identify' structure, as per ANSI ATA2 rev.2f spec */
{
	u16 Config, PhysCyls, Res2, PhysHeads, UnfBytesPerTrack;
	u16 UnfBytesPerSect, PhysSects, Vendor0, Vendor1, Vendor2;
	u8 SerialNum[20];
	u16 BufType, BufSize, ECCBytes;	u8 FirmwareRev[8], Model[40], MaxMult, Vendor3;	u16 DwordIO;
	u8 Vendor4, Capability;	u16 Res50;
	u8 Vendor5, PIOMode, Vendor6, DMAMode;	u16 LogValid, LogCyls, LogHeads, LogSects;
	u32 TotalSects;	u8 MultSect, MultSectValid;
	u32 LBASects;
	u16 DMAInfoSingle, DMAInfoMult, EIDEPIOModes;
	u16 EIDEDMAMin, EIDEDMATime, EIDEPIO, EIDEPIOIORdy;
	/* This fixed the drive-goes-north-after-ATAPI-identify bug
	u16 Res69, Res70; } ataid; */
	u16 Res[187];
} ataid;

/* generalized drive info structure */
typedef struct{
	u16 Flags;
	u8 DrvSel;
	/* ATA, ATAPI only (LUN for SCSI?) */
	u8 MultSect;
	/* ATA only */
	u16 Sects, Heads, Cyls;
	/* CHS ATA only */
	u16 IOAdr;
} drive;

drive Drive[4];

/*****************************************************************************
name:	ataSelect
*****************************************************************************/
int ataSelect(unsigned int IOAdr, unsigned char Sel)
{
	unsigned char Temp;
	Temp=inp(IOAdr + ATA_REG_DRVHD);
	if(((Temp ^ Sel) & 0x10) == 0) return(0); /* already selected */
	outp(IOAdr + ATA_REG_DRVHD, Sel);
	nsleep(400);
	for(Temp=WAIT_READY; Temp; Temp--)
	{
		if((inp(IOAdr + ATA_REG_STAT) & 0x80) == 0) break;
		msleep(1);
	}
	/* this _must_ be polled, I guess (sigh) */
	return(Temp == 0);
}

/*****************************************************************************
name:	ataProbe2
*****************************************************************************/

void ataProbe2(unsigned char WhichDrive, unsigned char DrvSel)
{
	unsigned char Temp, Temp1, Temp2;
	unsigned int IOAdr;
	ataid DriveInfo;

	IOAdr=Drive[WhichDrive].IOAdr;
	ataSelect(IOAdr, DrvSel);

	Temp1=inp(IOAdr + ATA_REG_CNT);
	Temp2=inp(IOAdr + ATA_REG_SECT);
	if (Temp1 != 0x01 || Temp2 != 0x01)
	{
		printf("nothing there\n");
		NO_DRIVE:
			Drive[WhichDrive].IOAdr=0;
		return;
	}
	Temp1=inp(IOAdr + ATA_REG_LOCYL);
	Temp2=inp(IOAdr + ATA_REG_HICYL);
	Temp=inp(IOAdr + ATA_REG_STAT);
	InterruptOccured=0;
	if (Temp1 == 0x14 && Temp2 == 0xEB)
	{
		printf("ATAPI CD-ROM, ");
		Drive[WhichDrive].Flags |= ATA_FLG_ATAPI;
		// issue ATAPI 'identify drive' command
		Temp1=ATA_CMD_PID;
		outp(IOAdr + ATA_REG_CMD, Temp1);
		Temp=(unsigned char)WAIT_PID;
	}
	else
	if(Temp1 == 0 && Temp2 == 0 && Temp)
	{
		printf("ATA hard drive, ");
		// issue ATA 'identify drive' command
		Temp1=ATA_CMD_ID;
		outp(IOAdr + ATA_REG_CMD, Temp1);
		Temp=(unsigned char)WAIT_ID;
	}
	else
	{
		printf("unknown drive type\n");
		goto NO_DRIVE;
	}
	// ATA or ATAPI: get results of of identify
	nsleep(400);
	if(awaitInterrupt(0xC000, Temp) == 0)
	// XXX - could be old drive that doesn't support 'identify'.Read geometry from partition table? Use (* gag *) CMOS?
	{
		printf("'identify' failed\n");
		goto NO_DRIVE;
	}
	// grab info returned by 'identify'
	(void)inp(IOAdr + ATA_REG_STAT);
	// for ATAPI CD-ROM, you MUST read 512 bytes here, ordrive will go comatose
	insw(IOAdr + ATA_REG_DATA, (u16 *)&DriveInfo, sizeof(DriveInfo) / 2);
	Temp2=1;
	if(Temp1 == ATA_CMD_PID)
	// model name is not byte swapped for NEC, Mitsumi, and Pioneer drives
	{
		if((DriveInfo.Model[0] == 'N' && DriveInfo.Model[1] == 'E') ||
			(DriveInfo.Model[0] == 'F' && DriveInfo.Model[1] == 'X') ||
			(DriveInfo.Model[0] == 'P' && DriveInfo.Model[1] == 'i'))
				Temp2=0;
	}
	for(Temp=0; Temp < 40; Temp += 2)
	{
		printf("%c", DriveInfo.Model[Temp ^ Temp2]);
		printf("%c", DriveInfo.Model[Temp ^ Temp2 ^ 1]);
	}
	printf("\n"
		"CHS=%u:%u:%u, ", DriveInfo.PhysCyls, DriveInfo.PhysHeads,
		DriveInfo.PhysSects);
	Drive[WhichDrive].Sects=DriveInfo.PhysSects;
	Drive[WhichDrive].Heads=DriveInfo.PhysHeads;
	Drive[WhichDrive].Cyls=DriveInfo.PhysCyls;
	if(DriveInfo.Capability & 1)
	{
		printf("DMA, ");
		Drive[WhichDrive].Flags |= ATA_FLG_DMA;
	}
	if(DriveInfo.Capability & 2)
	{
		printf("LBA, ");
		Drive[WhichDrive].Flags |= ATA_FLG_LBA;
	}
	// By Dobbs, I'll figure this out yet. Linux ide.c requires
	// (DriveInfo.MultSectValid & 1) && DriveInfo.MultSect
	// The magic value then comes from DriveInfo.MaxMult
	// QUANTUM FIREBALL ST2.1A			MaxMult=16	MultSect=16	MultSectValid=1
	// Conner Peripherals 850MB - CFS850A	MaxMult=16	MultSect=0	MultSectValid=1
	// (Seagate) st3144AT			MaxMult=0	MultSect=0	MultSectValid=0
	if((DriveInfo.MultSectValid & 1) && DriveInfo.MultSect)
	{
		Temp=DriveInfo.MaxMult;
		printf("MaxMult=%u, ", Temp);
	}
	else
		Temp=1;
	Drive[WhichDrive].MultSect=Temp;
	printf("%uK cache\n", DriveInfo.BufSize >> 1);
#ifdef DEBUG
	printf("\n"
		"Config=0x%hX, PhysCyls=%hu, Res2=%hu, PhysHeads=%hu, "
		"UnfBytesPerTrack=%hu\n"
		"PhysSects=%hu, Vendor0=%hu, Vendor1=%hu, "
		"Vendor2=%hu\n",DriveInfo.Config, DriveInfo.PhysCyls, DriveInfo.Res2, DriveInfo.PhysHeads,DriveInfo.UnfBytesPerTrack, DriveInfo.PhysSects, DriveInfo.Vendor0,DriveInfo.Vendor1, DriveInfo.Vendor2);
	printf("\n"
		"BufType=%hu, BufSize=%hu, ECCBytes=%hu, MaxMult=%hu, Vendor3=%hu, "
		"DwordIO=%hu\n"
		"Vendor4=%hu, Capability=0x%hX\n",DriveInfo.BufType, DriveInfo.BufSize, DriveInfo.ECCBytes, DriveInfo.MaxMult,DriveInfo.Vendor3, DriveInfo.DwordIO, DriveInfo.Vendor4, DriveInfo.Capability);
	printf("\n"
		"Res50=%hu, Vendor5=%hu, PIOMode=%hu, Vendor6=%hu, DMAMode=%hu, "
		"LogValid=%hu, LogCyls=%hu\n"
		"LogHeads=%hu, LogSects=%hu, ",DriveInfo.Res50, DriveInfo.Vendor5, DriveInfo.PIOMode, DriveInfo.Vendor6,DriveInfo.DMAMode, DriveInfo.LogValid, DriveInfo.LogCyls, DriveInfo.LogHeads,DriveInfo.LogSects);
	printf("TotalSects=%lu, MultSect=%hu\n",DriveInfo.TotalSects, DriveInfo.MultSect);printf("MultSectValid=%hu\n"
		"LBASects=%lu\n",DriveInfo.MultSectValid, DriveInfo.LBASects);
	printf("\n"
		"DMAInfoSingle=%hu, DMAInfoMult=%hu, EIDEPIOModes=%hu, EIDEDMAMin=%hu"
		"\n"
		"EIDEDMATime=%hu, EIDEPIO=%hu, EIDEPIOIORdy=%hu\n" "\n",DriveInfo.DMAInfoSingle, DriveInfo.DMAInfoMult, DriveInfo.EIDEPIOModes,DriveInfo.EIDEDMAMin, DriveInfo.EIDEDMATime, DriveInfo.EIDEPIO,DriveInfo.EIDEPIOIORdy);
#endif

}

char soft_reset(unsigned int IOAdr)
{
	unsigned char Temp;

	PRINT_DEBUG(printf("ataProbe: found something on I/F 0x%03X, "
			"doing soft reset...\n", IOAdr);)
	outp(IOAdr + ATA_REG_SLCT, 0x0E);
	nsleep(400);// release soft reset AND enable interrupts from drive
	outp(IOAdr + ATA_REG_SLCT, 0x08);
	nsleep(400);// wait for master
	for(Temp=2000; Temp; Temp--)	// XXX - why 2000?
	{
		if((inp(IOAdr + ATA_REG_STAT) & 0x80) == 0) break;
		msleep(1);
	}
	// XXX - blocking delay
	if(Temp == 0)
	{
		PRINT_DEBUG(printf("ataProbe: no master on I/F 0x%03X\n", IOAdr);)
		return -1;
	}
	return 0;
}

/*****************************************************************************
name:	ataProbe
*****************************************************************************/
void ataProbe(void)
{
	unsigned char Temp, Temp1, Temp2, WhichDrive;
	unsigned int IOAdr;

	printf("ataProbe:\n");
	/* set initial values */
	Drive[0].DrvSel=Drive[2].DrvSel=0xA0;
	Drive[1].DrvSel=Drive[3].DrvSel=0xB0;
	Drive[0].IOAdr=Drive[1].IOAdr=0x1F0;
	Drive[2].IOAdr=Drive[3].IOAdr=0x170;
	for (WhichDrive=0; WhichDrive < 4; WhichDrive += 2)
	{
		IOAdr=Drive[WhichDrive].IOAdr;
		/* poke at the interface to see if anything's there */
		PRINT_DEBUG(printf("ataProbe: poking interface 0x%03X\n", IOAdr);)
		outp(IOAdr + ATA_REG_CNT, 0x55);
		outp(IOAdr + ATA_REG_SECT, 0xAA);
		Temp1=inp(IOAdr + ATA_REG_CNT);
		Temp2=inp(IOAdr + ATA_REG_SECT);
		if(Temp1 != 0x55 || Temp2 != 0xAA)
		/* no master: clobber both master and slave */
		NO_DRIVES:
		{
			Drive[WhichDrive + 1].IOAdr=Drive[WhichDrive].IOAdr=0;
			continue;
		}
		/* soft reset both drives on this I/F (selects master) */
		if (!soft_reset(IOAdr)) { };
//			goto NO_DRIVES;
		/* identify master */
		printf("  hd%1u (0x%03X, master): ", WhichDrive, IOAdr);
		ataProbe2(WhichDrive, 0xA0);
		if (!soft_reset(IOAdr)) { };
//			goto NO_DRIVES;
		/* identify slave */
		printf("  hd%1u (0x%03X,  slave): ", WhichDrive + 1, IOAdr);
		ataProbe2(WhichDrive + 1, 0xB0);
	}
}

/*****************************************************************************
name:	ataCmd2
*****************************************************************************/
void ataCmd2(drivecmd *Cmd, u8 Count, u8 CmdByte)
{
	u8 Sect, DrvHd;	u16 Cyl, IOAdr;	u32 Temp;
	IOAdr=Drive[Cmd->Dev].IOAdr;

	/* compute CHS or LBA register values */
	Temp=Cmd->Blk;
	if(Drive[Cmd->Dev].Flags & ATA_FLG_LBA)
	{
		Sect=Temp;
		/* 28-bit sector adr: low byte */
		Cyl=Temp >> 8;
		/* middle bytes */
		DrvHd=Temp >> 24;
		/* high nybble */
		DrvHd=(DrvHd & 0x0F) | 0x40;/* b6 enables LBA */
		PRINT_DEBUG(printf("ataCmd2: LBA=%lu\n", Temp);)
	}
	else
	{
		Sect=Temp % Drive[Cmd->Dev].Sects + 1;
		Temp /= Drive[Cmd->Dev].Sects;
		DrvHd=Temp % Drive[Cmd->Dev].Heads;
		Cyl=Temp / Drive[Cmd->Dev].Heads;
		PRINT_DEBUG(printf("ataCmd2: CHS=%u:%u:%u\n", Cyl, DrvHd, Sect);)
	}
	DrvHd |= Drive[Cmd->Dev].DrvSel;
	PRINT_DEBUG(printf("ataCmd2: writing register file\n");)
	outp(IOAdr + ATA_REG_CNT, Count);
	outp(IOAdr + ATA_REG_SECT, Sect);
	outp(IOAdr + ATA_REG_LOCYL, Cyl);
	Cyl >>= 8; /* compiler bug work-around */
	outp(IOAdr + ATA_REG_HICYL, Cyl);
	// >> 8);
	outp(IOAdr + ATA_REG_DRVHD, DrvHd);
	InterruptOccured=0;
	outp(IOAdr + ATA_REG_CMD, CmdByte);
	nsleep(400);
}

/*****************************************************************************
name:	ataCmd
action:	ATA hard drive block read/write
returns: 0 if OK
	-1 if drive could not be selected
	-2 if unsupported command
	-3 if command timed out
	-4 if bad/questionable drive status after command
*****************************************************************************/
int ataCmd(drivecmd *Cmd)
{
	u8 Stat, CmdByte;
	u32 Count, Temp;
	u16 IOAdr;
	IOAdr=Drive[Cmd->Dev].IOAdr;
	/* select the drive */
	if(ataSelect(IOAdr, Drive[Cmd->Dev].DrvSel))
	{
		printf("ataCmd: could not select drive\n");
		return(-1);
	}
	if(Cmd->Cmd == DRV_CMD_RD)
	/* convert general block device command code into ATA command byte:ATA_CMD_RDMUL if drive supports multi-sector reads, ATA_CMD_RD if not */
	{
		if(Drive[Cmd->Dev].MultSect < 2)
			CmdByte=ATA_CMD_RD;
		else
			CmdByte=ATA_CMD_RDMUL;
		while(Cmd->Count)
		/* if drive supports multisector read/write, transfer as many sectors aspossible (fewer interrupts). We rely on MultSect to limit Temp (thesector count) to < 256 */
		{
			Temp=(Cmd->Count + ATA_SECTSIZE - 1) >> ATA_LG_SECTSIZE;
			Count=min(Temp, Drive[Cmd->Dev].MultSect);
			PRINT_DEBUG(printf("ataCmd: ready to read %lu "
					"sector(s) of %lu\n", Count, Temp);)
			/* compute CHS or LBA register values and write them, along with CmdByte */
			ataCmd2(Cmd, Count, CmdByte);/* await read interrupt */
			if(awaitInterrupt(0xC000, WAIT_CMD) == 0)
			{
				printf("ataCmd: read timed out\n");
				(void)inp(IOAdr + ATA_REG_STAT);
				return(-3);
			}
			/* check status */
			Stat=inp(IOAdr + ATA_REG_STAT);
			if((Stat & (0x81 | 0x58)) != 0x58)
			{
				printf("ataCmd: bad status (0x%02X) "
					"during read\n", Stat);
				return(-4);
			}
			/* advance pointers, read data */
			Cmd->Blk += Count;
			Count <<= ATA_LG_SECTSIZE;
			insw(IOAdr + ATA_REG_DATA, (u16 *)Cmd->Data, Count >> 1);
			Cmd->Data += Count;
			/* XXX - Cmd->Count had better be a multiple of 512... */
			Cmd->Count -= Count;
		}
		return(0);
	}
	else
	if(Cmd->Cmd == DRV_CMD_WR)
	/* convert general block device command code into ATA command byte:ATA_CMD_WRMUL if drive supports multi-sector reads, ATA_CMD_WR if not */
	{
		if(Drive[Cmd->Dev].MultSect < 2)
			CmdByte=ATA_CMD_WR;
		else
			CmdByte=ATA_CMD_WRMUL;
		while(Cmd->Count)
		/* if drive supports multisector read/write, transfer as many sectors aspossible (fewer interrupts). We rely on MultSect to limit Count(the sector count) to < 256 */
		{
			Temp=(Cmd->Count + ATA_SECTSIZE - 1) >> ATA_LG_SECTSIZE;
			Count=min(Temp, Drive[Cmd->Dev].MultSect);
			PRINT_DEBUG(printf("ataCmd: ready to write %lu "
					"sector(s) of %lu\n", Count, Temp);)
                        /* compute CHS or LBA register values and write them, along with CmdByte */
                        ataCmd2(Cmd, Count, CmdByte);
                        /* await DRQ */
		        for(Temp=WAIT_DRQ; Temp; Temp--)
		        {
			        if(inp(IOAdr + ATA_REG_ALTST) & 0x08) break;
				             msleep(1);
                        }
		        /* XXX - blocking delay */
		        if(Temp == 0)
		        {
			        printf("ataCmd: no DRQ during write\n");
			        (void)inp(IOAdr + ATA_REG_STAT);
			        return(-3);
                        }
		        /* advance pointer, write data */
		        Cmd->Blk += Count;
		        Count <<= ATA_LG_SECTSIZE;
		        outsw(IOAdr + ATA_REG_DATA, (u16 *)Cmd->Data, Count >> 1);
		        /* await write interrupt */
		        Temp=awaitInterrupt(0xC000, WAIT_CMD);
		        if(Temp == 0)
		        {
			        printf("ataCmd: write timed out\n");
			        (void)inp(IOAdr + ATA_REG_STAT);
			        return(-3);
		        }
		        /* check status */
		        Stat=inp(IOAdr + ATA_REG_STAT);
		        if((Stat & (0xA1 | 0x50)) != 0x50)
		        {
			        printf("ataCmd: bad status (0x%02X) during "
				"write\n", Stat);
			        return(-4);
		        }
		        /* advance pointers */
		        Cmd->Data += Count;
		        /* XXX - Cmd->Count had better be a multiple of 512... */
		        Cmd->Count -= Count;
                }
                return(0);
        }
        else
        {
	        printf("ataCmd: bad cmd (%u)\n", Cmd->Cmd);
	        return(-2);
        }
}
/*
////////////////////////////////////////////////////////////////////////////
ATAPI-ONLY STUFF
////////////////////////////////////////////////////////////////////////////
*/
/* ATAPI packet command bytes */
#define	ATAPI_CMD_START_STOP	0x1B	/* eject/load */
#define	ATAPI_CMD_READ10	0x28	/* read data sector(s) */
#define	ATAPI_CMD_READTOC	0x43	/* read audio table-of-contents */
#define	ATAPI_CMD_PLAY		0x47	/* play audio */
#define	ATAPI_CMD_PAUSE		0x4B	/* pause/continue audio */
/* ATAPI data/command transfer 'phases' */
#define	ATAPI_PH_ABORT		0	/* other possible phases (1, 2, */
#define	ATAPI_PH_DONE		3	/* and 11) are invalid */
#define	ATAPI_PH_DATAOUT	8
#define	ATAPI_PH_CMDOUT		9
#define	ATAPI_PH_DATAIN		10
typedef struct			/* 4 bytes */
{
	u8 Res0, Min, Sec, Frame;
} atapimsf;
typedef struct			/* 4 bytes */
{
	u16 TocLen;
	u8 FirstTrk;
	u8 LastTrk;
} atapitocheader;
typedef struct			/* 8 bytes */
{
	u8 Res0;
	/* In Linux ide.h, the next field was 'unsigned control : 4'I thought "hey, unsigned is 32 bits under GNU"... and that ledto great confusion. The size of this structure _IS_ 8 bytes. */
	u8 Ctrl : 4;
	u8 Adr : 4;
	u8 Trk;
	u8 Res1;
	union	{
		u32 Block;
		atapimsf Time;
	} Where;
} atapitocentry;
typedef struct
{
	atapitocheader Hdr;
	atapitocentry Ent[100];
} atapitoc;

/*****************************************************************************
name:	atapiCmd2
action:	writes ATA register file including packet command byte,
	busy-waits until drive ready, then writes 12-byte ATAPI
	command	packet, and services interrupts
returns:0  if success
	-1 if drive could not be selected
	-3 timeout after writing pkt cmd byte (0xA0)
	-4 timeout after writing cmd pkt
	-5 data shortage (premature ATAPI_PH_DONE)
	-6 drive aborted command
	-7 bad drive phase
*****************************************************************************/
/* ATA_REG_STAT & 0x08 (DRQ)
ATA_REG_REASON	"phase"	0	0
ATAPI_PH_ABORT		0	1
bad			0	2
bad			0	3
ATAPI_PH_DONE		8	0
ATAPI_PH_DATAOUT	8	1
ATAPI_PH_CMDOUT		8	2
ATAPI_PH_DATAIN		8	3
badb0 of ATA_REG_REASON is C/nD (0=data, 1=command)b1 of ATA_REG_REASON is IO (0=out to drive, 1=in from drive) */
int atapiCmd2(drivecmd *Cmd, u8 *Pkt)
{
	u16 IOAdr, Got;
	u8 Phase;
	u32 Temp;
	IOAdr=Drive[Cmd->Dev].IOAdr;
	if(ataSelect(IOAdr, Drive[Cmd->Dev].DrvSel))
	{
		printf("atapiCmd2: could not select drive\n");
		return(-1);
	}
	PRINT_DEBUG(printf("atapiCmd2: writing register file\n");)
	outp(IOAdr + ATA_REG_FEAT, 0);
	/* irrelevant? */
	outp(IOAdr + ATA_REG_REASON, 0);
	/* irrelevant? */
	outp(IOAdr + ATA_REG_SECT, 0);
	/* irrelevant? */
	outp(IOAdr + ATA_REG_LOCNT, 32768ul);
	outp(IOAdr + ATA_REG_HICNT, 32768ul >> 8);
	outp(IOAdr + ATA_REG_CMD, ATA_CMD_PKT);
	nsleep(400);
	for(Temp=500; Temp; Temp--)
	/* XXX - why 500? */
	{
		if((inp(IOAdr + ATA_REG_STAT) & 0x88) == 0x08) break;
		msleep(1);
	}
	/* this _must_ be polled, I guess */
	if(Temp == 0)
	{
		printf("atapiCmd2: drive did not accept pkt cmd byte\n");
		return(-3);
	}
	InterruptOccured=0;
	outsw(IOAdr + ATA_REG_DATA, (u16 *)Pkt, 6);
	while(1)
	{
		if(awaitInterrupt(0xC000, WAIT_CMD) == 0)
		{
			(void)inp(IOAdr + ATA_REG_STAT);
			printf("atapiCmd2: pkt cmd timed out\n");
			return(-4);
		}
		Phase=inp(IOAdr + ATA_REG_STAT) & 0x08;
		Phase |= (inp(IOAdr + ATA_REG_REASON) & 3);
		if(Phase == ATAPI_PH_DONE)
		{
			if(Cmd->Count)	
			{
				printf("atapiCmd2: data shortage\n");/* could mean no CD or audio CD */
				return(-5);
			}
			return(0);
		}
		else
		if(Phase == ATAPI_PH_ABORT)
		{
			printf("atapiCmd2: cmd aborted\n");
			return(-6);
		}
		else
		if(Phase != ATAPI_PH_DATAIN)/* ATAPI_PH_DATAOUT or ATAPI_PH_CMDOUT or something completely bogus */
		{
			printf("atapiCmd2: bad phase %u\n", Phase);
			return(-7);
		}
		Got=inp(IOAdr + ATA_REG_HICNT);
		Got=(Got << 8) | inp(IOAdr + ATA_REG_LOCNT);
		PRINT_DEBUG(printf("<%5u bytes>   ", Got);)
		/* Cmd->Count=how many bytes we want to transferGot=how many bytes are available for transferTemp=smaller of these two */	
		Temp=min(Cmd->Count, Got);
		/* read data. XXX - what if Temp odd?XXX - allow read from middle of one sector to middle of another? */
		insw(IOAdr + ATA_REG_DATA, (u16 *)Cmd->Data, Temp >> 1);
		/* read and discard surplus data */
		for(Got -= Temp; Got > 1; Got -= 2)
			inpw(IOAdr + ATA_REG_DATA);
		if(Got) inp(IOAdr + ATA_REG_DATA);
		/* advance pointers */
		Cmd->Data += Temp;
		Cmd->Count -= Temp;
		Temp >>= ATAPI_LG_SECTSIZE;
		Cmd->Blk += Temp;
	}
}

/*****************************************************************************
name:	atapiCmd
action:	ATAPI device block read (and later, write)
returns:0  if success (OK)
	-1 if drive could not be selected
	-2 if unsupported command
	-3 timeout after writing pkt cmd byte (0xA0)
	-4 timeout after writing cmd pkt
	-5 data shortage (premature ATAPI_PH_DONE)
	-6 drive aborted command
	-7 bad drive phase
*****************************************************************************/
int atapiCmd(drivecmd *Cmd)
{
	u8 Pkt[12];
	u32 Count;
	/* convert general block device command code into ATAPI packet commands */
	if(Cmd->Cmd == DRV_CMD_RD)
		Pkt[0]=ATAPI_CMD_READ10;
	else
	{
		printf("atapiCmd: bad cmd\n");
		return(-2);
	}
	Pkt[1]=0;
	Pkt[2]=Cmd->Blk >> 24;
	Pkt[3]=Cmd->Blk >> 16;
	Pkt[4]=Cmd->Blk >> 8;
	Pkt[5]=Cmd->Blk;
	Count=Cmd->Count >> ATAPI_LG_SECTSIZE;
	Pkt[6]=Count >> 16;
	Pkt[7]=Count >> 8;
	Pkt[8]=Count;
	Pkt[9]=Pkt[10]=Pkt[11]=0;
	return(atapiCmd2(Cmd, Pkt));
}

/*****************************************************************************
name:	atapiEject
action:	loads (Load == 0) or ejects (Load != 0) CD-ROM
returns:whatever atapiCmd2() returns
*****************************************************************************/
int atapiEject(unsigned Load, unsigned WhichDrive)
{
	char Pkt[12]={
		ATAPI_CMD_START_STOP,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0 };
	drivecmd Cmd;
	PRINT_DEBUG(printf("atapiEject\n");)
	Cmd.Blk=0;
	Cmd.Count=0;
	Cmd.Dev=WhichDrive;
	Cmd.Cmd=0;
	Cmd.Data=NULL;
	Pkt[4]=2 + (Load != 0);
	return(atapiCmd2(&Cmd, Pkt));
}

/*****************************************************************************
name:	atapiPlay
action:	plays audio from time index Start to End (units of 1/75 sec)
returns:whatever atapiCmd2() returns
*****************************************************************************/
int atapiPlay(unsigned WhichDrive, u32 Start, u32 End)
{
	char Pkt[12]={
		ATAPI_CMD_PLAY,
		0, 0,
		0, 0, 0,	/* starting minute:second:frame */
		0, 0, 0,	/* ending M:S:F (frame=1/75 sec) */
		0, 0, 0 };
	drivecmd Cmd;
	PRINT_DEBUG(printf("atapiPlay\n");)
	Cmd.Blk=0;
	Cmd.Count=0;
	Cmd.Dev=WhichDrive;
	Cmd.Cmd=0;
	Cmd.Data=NULL;
	Pkt[5]=Start % 75;
	Start /= 75;
	Pkt[4]=Start % 60;
	Start /= 60;
	Pkt[3]=Start;
	Pkt[8]=End % 75;
	End /= 75;
	Pkt[7]=End % 60;
	End /= 60;
	Pkt[6]=End;
	return(atapiCmd2(&Cmd, Pkt));
}

/*****************************************************************************
name:	atapiTOCEnt
action:	reads one or more table-of-contents entries from audio CD
returns:whatever atapiCmd2() returns
*****************************************************************************/
int atapiTOCEnt(atapitoc *Contents, unsigned Count, unsigned WhichDrive)
{
	char Pkt[12]={
		ATAPI_CMD_READTOC,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0 };
	drivecmd Cmd;
	PRINT_DEBUG(printf("atapiTOCEnt\n");)
	Cmd.Blk=0;
	Cmd.Count=Count;
	Cmd.Dev=WhichDrive;
	Cmd.Cmd=0;
	Cmd.Data=(u8 *)Contents;
	Pkt[1]=2;
	Pkt[6]=0;
	Pkt[7]=Count >> 8;
	Pkt[8]=Count;
	Pkt[9]=0;
	return(atapiCmd2(&Cmd, Pkt));
}

/*****************************************************************************
name:	atapiTOC
action:	reads table of contents of audio CD and prints starting	
	time of each track
returns:whatever atapiCmd2() returns
*****************************************************************************/
#define	MAX_TRACKS		32
atapimsf Track[MAX_TRACKS];
unsigned NumTracks;
int atapiTOC(unsigned WhichDrive)
{
	atapitoc Contents;
	int Temp;/* read just the header at first */
	Temp=sizeof(atapitocheader);
	PRINT_DEBUG(printf("atapiTOC: calling atapiTOCEnt with Count=%u\n", Temp);)
	Temp=atapiTOCEnt(&Contents, Temp, WhichDrive);
	if(Temp)
		return(Temp);
	NumTracks=Contents.Hdr.LastTrk - Contents.Hdr.FirstTrk + 1;
	if(NumTracks <= 0 || NumTracks > 99)
	{
		printf("atapiTOC: bad number of tracks %d\n", NumTracks);
		return(-1);
	}
	if(NumTracks > MAX_TRACKS)
	{
		printf("Too many tracks (%u); reducing to %u.\n", NumTracks, MAX_TRACKS);
		NumTracks=MAX_TRACKS;
	}
	Temp=sizeof(atapitocheader) + (NumTracks + 1) * sizeof(atapitocentry);
	PRINT_DEBUG(printf("atapiTOC: calling atapiTOCEnt with Count=%u\n", Temp);)
	Temp=atapiTOCEnt(&Contents, Temp, WhichDrive);
	if(Temp)
		return(Temp);
	for(Temp=0; Temp <= NumTracks; Temp++)
	{
		Track[Temp].Min=Contents.Ent[Temp].Where.Time.Min;
		Track[Temp].Sec=Contents.Ent[Temp].Where.Time.Sec;
		Track[Temp].Frame=Contents.Ent[Temp].Where.Time.Frame;
		printf("%02u:%02u:%02u  ", Contents.Ent[Temp].Where.Time.Min, Contents.Ent[Temp].Where.Time.Sec, Contents.Ent[Temp].Where.Time.Frame);
	}
	printf("\n");
	return(0);
}

/*****************************************************************************
name:	atapiPause
action:	pauses (Play == 0) or continues (Play != 0) audio CD
returns:whatever atapiCmd2() returns
*****************************************************************************/
int atapiPause(unsigned Play, unsigned WhichDrive)
{
	char Pkt[12]={
		ATAPI_CMD_PAUSE,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0 };
	drivecmd Cmd;
	PRINT_DEBUG(printf("atapiPause\n");)
	Cmd.Blk=0;
	Cmd.Count=0;
	Cmd.Dev=WhichDrive;
	Cmd.Cmd=0;
	Cmd.Data=NULL;
	Pkt[8]=(Play != 0);
	return(atapiCmd2(&Cmd, Pkt));
}

/*
////////////////////////////////////////////////////////////////////////////
GENERAL PARTITION STUFF
////////////////////////////////////////////////////////////////////////////
*/
/*****************************************************************************
name:	partProbe
action:	analyzes partitions on ATA drives
*****************************************************************************/
void partProbe(void)
{
	u8 Buffer[512], WhichDrive, WhichPart;
	unsigned HiHead, Scale, LHeads, Heads, Sects;
	long int Temp, Track;
	unsigned Offset, Cyls;
	drivecmd Cmd;
	printf("partProbe:\n");
	for(WhichDrive=0; WhichDrive < 4; WhichDrive++)
	{
		if(Drive[WhichDrive].IOAdr == 0) continue;
		if(Drive[WhichDrive].Flags & ATA_FLG_ATAPI) continue;
		/* load sector 0 (the partition table) */
		Cmd.Blk=0;
		Cmd.Count=512;
		Cmd.Dev=WhichDrive;
		Cmd.Cmd=DRV_CMD_RD;
		Cmd.Data=Buffer;
		Temp=ataCmd(&Cmd);
		if(Temp < 0) continue;
		/* see if it's valid */
		if(Buffer[0x1FE] != 0x55 || Buffer[0x1FF] != 0xAA)
		{
			printf("  hd%1u: invalid partition table\n", WhichDrive);
			continue;
		}
		/* check all four primary partitions for highest Heads value */
		HiHead=0;
		for(WhichPart=0; WhichPart < 4; WhichPart++)
		{
			Offset=0x1BE + 16 * WhichPart;
			if(Buffer[Offset + 1] > HiHead)
				HiHead=Buffer[Offset + 1];
			if(Buffer[Offset + 5] > HiHead)
				HiHead=Buffer[Offset + 5];
		}
		/* compare highest head value with heads/cylinder value from 'identify'.Check for LARGE mode and determine Scale. This test will fail unlesspartitions end on a cylinder boundary (hopefully, they do). */
		HiHead++;
		LHeads=Drive[WhichDrive].Heads;
		if(HiHead > LHeads)
		{
			Scale=HiHead / LHeads;
			printf("  hd%1u: LARGE mode, N=", WhichDrive);
			if(Scale * LHeads == HiHead)
			{
				LHeads *= Scale;
				printf("%u, new CHS=%u:%u:%u\n", Scale,	Drive[WhichDrive].Cyls / Scale,	LHeads,	Drive[WhichDrive].Sects);
			}
			/* HiHead / Drive[WhichDrive].Heads is not an integer. */
			else
				printf("??? (UNKNOWN !!!)\n");
		}
		/* now print geometry info for all primary partitions. CHS values in eachpartition record may be faked for the benefit of MS-DOS/Win, so we ignorethem and use the 32-bit size-of-partition and sectors-preceding-partitionfields to compute CHS */
		for(WhichPart=0; WhichPart < 4; WhichPart++)
		{
			Offset=0x1BE + 16 * WhichPart;
			/* get 32-bit sectors-preceding-partition; skip if undefined partition */
			Temp=*(u32 *)(Buffer + Offset + 8);
			if(Temp == 0) continue;
			/* convert to CHS, using LARGE mode value of Heads if necessary */
			Sects=Temp % Drive[WhichDrive].Sects + 1;
			Track=Temp / Drive[WhichDrive].Sects;
			Heads=Track % LHeads;
			Cyls=Track / LHeads;
			printf("  hd%1u%c: start LBA=%8lu, start CHS=%4u:"
				"%2u:%2u, ", WhichDrive, 'a' + WhichPart, Temp, Cyls, Heads, Sects);
			/* get 32-bit partition size */
			Temp=*(u32 *)(Buffer + Offset + 12);
			printf("%lu sectors\n", Temp);
		}
	}
}
/*
////////////////////////////////////////////////////////////////////////////
IRQ, DEMO, AND MAIN ROUTINES
////////////////////////////////////////////////////////////////////////////
*/
#define		IRQ14_VECTOR		118
#define		IRQ15_VECTOR		119
#define		CHAIN_INTS
//#define		BUFF_SIZE		49152u
#define		BUFF_SIZE		16384
drivecmd Cmd;
char Buffer[BUFF_SIZE];
#if defined(__DJGPP__)
	#include <dpmi.h>
	#include <go32.h>
	#include <crt0.h>
	_go32_dpmi_seginfo OldIRQ14Vector;
	_go32_dpmi_seginfo OldIRQ15Vector;
	/*****************************************************************************
	name:	irq14
	action:	IRQ 14 handler
	*****************************************************************************/
	void irq14(void)
	{
		InterruptOccured |= 0x4000;
		outp(0xA0, 0x20);
		nsleep(1000);
		outp(0x20, 0x20);
	}
	/*****************************************************************************
	name:	irq15
	action:	IRQ 15 handler
	*****************************************************************************/
	void irq15(void)
	{
		InterruptOccured |= 0x8000;
		outp(0xA0, 0x20);
		nsleep(1000);
		outp(0x20, 0x20);
	}
	/*****************************************************************************
	name:	irqStart
	action:	saves old interrupt vectors, installs new handlers
	*****************************************************************************/
	void irqStart(void)
	{
		_go32_dpmi_seginfo NewVector;
		/* lock memory (disable paging) */
		_crt0_startup_flags |= _CRT0_FLAG_LOCK_MEMORY;
		_go32_dpmi_get_protected_mode_interrupt_vector(IRQ14_VECTOR, &OldIRQ14Vector);
		_go32_dpmi_get_protected_mode_interrupt_vector(IRQ15_VECTOR, &OldIRQ15Vector);
		NewVector.pm_selector=_my_cs();
		NewVector.pm_offset=irq14;
		/* ptr -> int */
		_go32_dpmi_allocate_iret_wrapper(&NewVector);
		#if defined(CHAIN_INTS)
			_go32_dpmi_chain_protected_mode_interrupt_vector(IRQ14_VECTOR,	&NewVector);
		#endif
		_go32_dpmi_set_protected_mode_interrupt_vector(IRQ14_VECTOR, &NewVector);
		NewVector.pm_selector=_my_cs();
		NewVector.pm_offset=irq15;
		/* ptr -> int */
		_go32_dpmi_allocate_iret_wrapper(&NewVector);
		#if defined(CHAIN_INTS)
			_go32_dpmi_chain_protected_mode_interrupt_vector(IRQ15_VECTOR,	&NewVector);
		#endif
		_go32_dpmi_set_protected_mode_interrupt_vector(IRQ15_VECTOR, &NewVector);
	}
	/*****************************************************************************
	name:	irqEnd
	action:	restores old IRQ handlers
	*****************************************************************************/
	void irqEnd(void)
	{
		_go32_dpmi_set_protected_mode_interrupt_vector(IRQ14_VECTOR, &OldIRQ14Vector);
		_go32_dpmi_set_protected_mode_interrupt_vector(IRQ15_VECTOR, &OldIRQ15Vector);
	}
#elif defined(__BORLANDC__)
	void interrupt (*OldIRQ14Vector)(void);
	void interrupt (*OldIRQ15Vector)(void);
	/*****************************************************************************
	name:	irq14
	action:	IRQ 14 handler
	*****************************************************************************/
	void interrupt irq14(void)
	{
		InterruptOccured |= 0x4000;
		#if defined(CHAIN_INTS)
			_chain_intr(OldIRQ14Vector);
		#else
			outp(0xA0, 0x20);
			nsleep(1000);
			outp(0x20, 0x20);
		#endif
	}
	/*****************************************************************************
	name:	irq15
	action:	IRQ 15 handler
	*****************************************************************************/
	void interrupt irq15(void)
	{
		InterruptOccured |= 0x8000;
		#if defined(CHAIN_INTS)
			_chain_intr(OldIRQ15Vector);
		#else
			outp(0xA0, 0x20);
			nsleep(1000);
			outp(0x20, 0x20);
		#endif
	}
	/*****************************************************************************
	name:	ctrlBrk
	action:	Ctrl-Brk handler
		Does not return, but calls exit() instead.
	*****************************************************************************/
	int ctrlBrk(void)
	{
		printf("*** CtrlBrk pressed ***\n");
		setvect(IRQ14_VECTOR, OldIRQ14Vector);
		setvect(IRQ15_VECTOR, OldIRQ15Vector);
		return(0);
	}
	/*****************************************************************************
	name:	irqStart
	action:	saves old interrupt vectors, installs new handlers
	*****************************************************************************/
	void irqStart(void)
	{
		ctrlbrk(ctrlBrk);
		OldIRQ14Vector=getvect(IRQ14_VECTOR);
		OldIRQ15Vector=getvect(IRQ15_VECTOR);
		setvect(IRQ14_VECTOR, irq14);
		setvect(IRQ15_VECTOR, irq15);
	}
	/*****************************************************************************
	name:	irqEnd
	action:	restores old IRQ handlers
	*****************************************************************************/
	void irqEnd(void)
	{
		setvect(IRQ14_VECTOR, OldIRQ14Vector);
		setvect(IRQ15_VECTOR, OldIRQ15Vector);
	}
#else
	#error Not DJGPP, not Borland C. Sorry.
#endif
/*****************************************************************************
name:	demo
action:	loads interesting sectors from ATA and ATAPI drives
	and dumps them in hex
*****************************************************************************/
void demo(void)
{
	unsigned Count, WhichDrive;
	for(WhichDrive=0; WhichDrive < 4; WhichDrive++)
	{
		if(Drive[WhichDrive].IOAdr == 0) continue;
		Cmd.Dev=WhichDrive;
		Cmd.Cmd=DRV_CMD_RD;
		Cmd.Data=Buffer;
		if(Drive[WhichDrive].Flags & ATA_FLG_ATAPI)
		/* CD-ROM has it's root directory at logical block #16. According toISO-9660, the first 16 blocks (2048 bytes each) are all zeroes. (Theseblocks might now be used for El Torrito boot-from-CD.) */
		{
			Count=17;
			Cmd.Blk=17 - Count;
			Cmd.Count=Count << ATAPI_LG_SECTSIZE;
			printf("atapiCmd() returned %d\n", atapiCmd(&Cmd));
			dump(Buffer + ATAPI_SECTSIZE * (Count - 1), 96);
		}
		else
		/* load first sector of first partition, plus the MultSect sectors before it,to demonstrate multisector read (if supported) */
		{
			Count=Drive[WhichDrive].MultSect + 1;
			Cmd.Blk=Drive[WhichDrive].Sects + 1 - Count;
			Cmd.Count=Count << ATA_LG_SECTSIZE;
			printf("ataCmd() returned %d\n", ataCmd(&Cmd));	
			dump(Buffer + ATA_SECTSIZE * (Count - 1), 96);
		}
	}
}

/*****************************************************************************
name:	main
*****************************************************************************/
int main(void)
{
	unsigned char Data, WhichDrive, Temp;
	long Blk, Count;

	irqStart();
	/* enable IRQ14 and IRQ15 at the 2nd 8259 PIC chip */
	outp(0xA1, inp(0xA1) & ~0xC0);
	ataProbe();
	partProbe();
#ifdef DEMO
	/* atapiCmd demonstrates data CDs */
	demo();
#else
	/* atapiTOC and atapiPlay demonstrate audio CDs */
	for(WhichDrive=0; WhichDrive < 4; WhichDrive++)
	{
		if(Drive[WhichDrive].IOAdr && (Drive[WhichDrive].Flags & ATA_FLG_ATAPI))
			break;
	}
	if(WhichDrive == 4)
	{
		printf("No ATAPI devices detected.\n");
		goto BAIL;
	}
	printf("\n");
	atapiTOC(WhichDrive);
	for(Count=3; Count; Count--)
	/* play from 13:11:32 to 18:01:00 -- Zeppelin */
	{
		Temp=atapiPlay(WhichDrive, (13ul * 60 + 11) * 75 + 32,	(18ul * 60 + 1) * 75 + 0);
		if(Temp)
			printf("atapiPlay returned error %d, "
				"retrying...\n", Temp);
		else
			 break;
	}
#endif
#ifdef ERASE
	#define		START_BLK		2048ul
	#define		NUM_BLKS		8192ul
	printf(	"\n"
		"********************************************************\n"
		"***                      DANGER!                     ***\n"
		"********************************************************\n"
		"THIS OPERATION WILL ERASE PORTIONS OF YOUR HARD DRIVE!!!\n"
		 "\n"
		"Press 'y' to continue.\n"
		"\x07\x07\x07");
	Temp=getch();
	if(Temp != 'y' && Temp !='Y') goto BAIL;
	printf(	"\n"
		"********************************************************\n"
		"***                 SECOND WARNING!                  ***\n"
		"********************************************************\n"
		"Are you sure you want to ERASE YOUR HARD DRIVE?\n"
		"\n"		"Press 'p' to proceed.\n"
		"\x07");
	Temp=getch();
	if(Temp != 'p' && Temp != 'P') goto BAIL;
	Cmd.Dev=0;
	srand(2);
	Cmd.Cmd=DRV_CMD_WR;
	/* write NUM_BLKS of pseudorandom data to beginning of drive. SkipsSTART_BLK sectors, which may or may not leave the FAT, DOS kernel files,and COMMAND.COM intact. THIS ERASES DATA ON THE DRIVE!!! */
	for(Blk=START_BLK; Blk < START_BLK + NUM_BLKS; )
	{
		for(Temp=0; Temp < BUFF_SIZE; Temp++)
			Buffer[Temp]=rand();
		printf("writing Blk %lu\r", Blk);
		Cmd.Data=Buffer;
		Cmd.Blk=Blk;
		Count=min(BUFF_SIZE >> 9, START_BLK + NUM_BLKS - Blk);
		Cmd.Count=Count << 9;
		Temp=ataCmd(&Cmd);
		if(Temp)
		{
			printf("\n"
				"ataCmd(write) returned %d\n", Temp);
			break;
		}
		Blk += Count;
	}
	printf("\n");
	srand(2);
	Cmd.Cmd=DRV_CMD_RD;
	/* read NUM_BLKS of data from beginning of drive, verify against samepseudorandom sequence that was written */
	for(Blk=START_BLK; Blk < START_BLK + NUM_BLKS; )
	{
		printf("reading Blk %lu\r", Blk);
		Cmd.Data=Buffer;
		Cmd.Blk=Blk;
		Count=min(BUFF_SIZE >> 9, START_BLK + NUM_BLKS - Blk);
		Cmd.Count=Count << 9;
		Temp=ataCmd(&Cmd);
		if(Temp)
		{
			printf("\n"
				"ataCmd(read) returned %d\n", Temp);	
			break;
		}
		for(Temp=0; Temp < min(BUFF_SIZE, (START_BLK + NUM_BLKS - Blk) << 9); Temp++)
		{
			Data=rand();
			if(Buffer[Temp] != Data)
			{
				printf("\n"
					"verify failed, Blk %lu, offset "
					"%u (wanted %u, got %u)\n", Blk, Temp, Data, Buffer[Temp]);
				break;
			}
		}
		Blk += Count;
	}
#endif
BAIL:
	printf("\n");
	irqEnd();
	return 0;
}