/*
* Floppy access					*
* v0.1: TDS						*
*       - basic routines		*
* v0.2: TDS (30.03.2004)		*
*       - floppy read/write		*
*       - still buggy DMA code	*
*       - still buggy :-/		*
* v0.3: TDS (26.05.2004)		*
*		- #define TBADDR fixes	*
*		  DMA problem			*
* v0.4: TDS (09.08.2004)		*
*		- now supports up to 2	*
*		  drives still with 	*
*		  problems in detection	*
* v0.5: Doug Gale				*
*		- Fixed CMOS access		*
*		- Relocated DMA buffer	*
*/

#include <stdio.h>
#include <support.h>
#include <datatypes.h>
#include <drivers/dma.h>
#include <drivers/block/floppy.h>
#include <drivers/cmos.h>
#include <drivers/timer.h>
#include <multi.h>
#include <drivers/mem/mem.h>
#include <drivers/block/disk.h>

#include <irqa.h>
#include <interrupts.h>
#include <drivers/pic.h>

#define TBADDR 0x90000    /* physical address of track buffer located below 1M */

/* globals */
volatile BOOL done = FALSE;	// volatile, cos it's changed by an interrupt
int mtick = 0;
dword tmout = 0;
struct floppy_t floppies[2];
unsigned char fdc = 0;

#define floppy (&floppies[fdc])

void floppy_sendbyte(int byte);
int  floppy_getbyte(void);
BOOL floppy_wait(BOOL sensei);
BOOL floppy_read_block(int block, byte *blockbuff, unsigned long nosectors);
BOOL floppy_rw(int block, byte *blockbuff, BOOL read, dword nosectors, BOOL firsttry);
BOOL floppy_seek(int track);
void floppy_recalibrate(void);

/* helper functions */
// Called by IRQ handler
void floppy_process(void)
{
   /* signal operation finished */
   done = TRUE;
   return;
}

void floppy_update(void)
{
	if (tmout)
		tmout--;     /* bump timeout */
   
	if (mtick > 0)
		mtick--;
	else
	if (!mtick && floppy->motor)
	{
		outportb(floppy->FDC_DOR, 0x00);  /* turn off floppy motor, maybe 0x0C? */
		floppy->motor = FALSE;
	}	
}

/*floppy_sendbyte() routine from intel manual */
void floppy_sendbyte(int what)
{
	volatile int msr;
	int tmo;
   
	for (tmo = 0;tmo < 128;tmo++)
	{
		msr = inportb(floppy->FDC_MSR);
		if ((msr & 0xc0) == 0x80)
		{
			outportb(floppy->FDC_DATA, what);
			return;
		}
		inportb(0x80);   /* delay */
	}
}

/* getbyte() routine from intel manual */
int floppy_getbyte(void)
{
	volatile int msr;
	int tmo;
   
	for (tmo = 0;tmo < 128;tmo++)
	{
		msr = inportb(floppy->FDC_MSR);
		if ((msr & 0xd0) == 0xd0)
		{
			return inportb(floppy->FDC_DATA);
		}
		inportb(0x80);   /* delay */
	}
	return -1;   /* read timeout */
}

/* this waits for FDC command to complete */
BOOL floppy_wait(BOOL sensei)
{
	tmout = timer_ticks + 100;
     
	/* wait for IRQ6 handler to signal command finished */
	while (!done && (tmout > timer_ticks))
		multi_yield();

	/* read in command result bytes */
	floppy->statsz = 0;
	while ((floppy->statsz < 7) && (inportb(floppy->FDC_MSR) & (1<<4)))
	{
		floppy->status[floppy->statsz++] = floppy_getbyte();
	}

	if (sensei)
	{
		/* send a "sense interrupt status" command */
		floppy_sendbyte(CMD_SENSEI);
		floppy->sr0 = floppy_getbyte();
		floppy->track = floppy_getbyte();
	}
	done = FALSE;
   
	if (!tmout)
	{
		/* timed out! */
		if (inportb(floppy->FDC_DIR) & 0x80)  /* check for diskchange */
			floppy->dchange = TRUE;
		return FALSE;
	}
	else
		return TRUE;
}

/*
 * converts linear block address to head/track/sector
 * 
 * blocks are numbered 0..heads*tracks*sectors-1
 * blocks 0..sectors-1 are serviced by head #0
 * blocks sectors..sectors*2-1 are serviced by head 1
 * 
 * WARNING: garbage in == garbage out
 */
void block2hts(int block, int *head, int *track, int *sector)
{
	if (floppy->geometry.sectors == 0)
	{
		dprintf("floppy: sectors can't be zero!\n");
		return;
	}	
	*head = ((block % (floppy->geometry.sectors * floppy->geometry.heads)) / (floppy->geometry.sectors));
	*track = (block / (floppy->geometry.sectors * floppy->geometry.heads));
	*sector = ((block % floppy->geometry.sectors) + 1);
}

/**** disk operations ****/

/* this gets the FDC to a known state */
void floppy_reset(void)
{
   /* stop the motor and disable IRQ/DMA */
	outportb(floppy->FDC_DOR, 0x00);
	outportb(floppy->FDC_DOR, 0x0C);
	outportb(floppy->FDC_DOR, 0x00);
	delay(100); // give motor some time to stop
   
	mtick = 0;
	floppy->motor = FALSE;

   /* program data rate (500K/s) */
	outportb(floppy->FDC_DRS, 0);

   /* re-enable interrupts */
	outportb(floppy->FDC_DOR,0x0c);

   /* resetting triggered an interrupt - handle it */
	done = TRUE;
	floppy_wait(TRUE);

   /* specify drive timings (got these off the BIOS) */
	floppy_sendbyte(CMD_SPECIFY);
	floppy_sendbyte(0xdf);  /* SRT = 3ms, HUT = 240ms */
	floppy_sendbyte(0x02);  /* HLT = 16ms, ND = 0 */
   
   /* clear "disk change" status */
	floppy_seek(1);
	floppy_recalibrate();

	floppy->dchange = FALSE;
}

void floppy_status(void)
{
	char * status[] = {
    	"floppy drive 0 in seek mode/busy",
    	"floppy drive 1 in seek mode/busy",
    	"floppy drive 2 in seek mode/busy",
    	"floppy drive 3 in seek mode/busy",
    	"FDC read or write command in progress",
    	"FDC is in non-DMA mode",
    	"I/O direction;  1 = FDC to CPU; 0 = CPU to FDC",
    	"data reg ready for I/O to/from CPU (request for master)"};
	int st = inportb(floppy->FDC_MSR);
	int i;
	
	printf("status: %X\n", st);
	for (i=0; i<8; i++)
    {
		if (st & (1 << i))
		{
			printf("test %d == 0x%x (", i, 1<<i);
			printf("%s)\n", status[i]);
		}
	}
}

/* this returns whether there was a disk change */
BOOL floppy_diskchange(void)
{
	return floppy->dchange;
}

/* this turns the motor on */
void floppy_motoron(void)
{
	if (!floppy->motor)
	{
		mtick = -1;     /* stop motor kill countdown */
		outportb(floppy->FDC_DOR, 0x1C);
		delay(500); /* delay 500ms for motor to spin up */
		floppy->motor = TRUE;
	}
	return;
}

/* this turns the motor off */
void floppy_motoroff(void)
{
   if (floppy->motor) {
      mtick = 13500;   /* start motor kill countdown: 36 ticks ~ 2s */
   }
   return;
}

/* recalibrate the drive */
void floppy_recalibrate(void)
{
   /* turn the motor on */
	floppy_motoron();
   
   /* send actual command bytes */
	floppy_sendbyte(CMD_RECAL);
	floppy_sendbyte(0);

   /* wait until seek finished */
	floppy_wait(TRUE);
   
   /* turn the motor off */
	floppy_motoroff();
}

/* seek to track */
BOOL floppy_seek(int track)
{
	if (floppy->track == track)  /* already there? */
		return TRUE;
   
	floppy_motoron();
   
   /* send actual command bytes */
	floppy_sendbyte(CMD_SEEK);
	floppy_sendbyte(0);
	floppy_sendbyte(track);

   /* wait until seek finished */
	if (!floppy_wait(TRUE))
		return FALSE;     /* timeout! */
	/* now let head settle for 15ms */
	delay(15);
   
	floppy_motoroff();
   
   /* check that seek worked */
	if ((floppy->sr0 != 0x20) || (floppy->track != track))
		return FALSE;
	else
		return TRUE;
}

/* checks drive geometry - call this after any disk change */
BOOL floppy_get_geo(void)
{
	char tmp[512];
	
	/* get drive in a known status before we do anything */
	floppy_reset();

	/* assume disk is 1.68M and try and read block #21 on first track */
	floppy->geometry.cyls = DG168_TRACKS;
	floppy->geometry.heads = DG168_HEADS;
	floppy->geometry.sectors = DG168_SECTORS;
	dprintf("floppy: Checking 1.68MB disk...\n");
	if (floppy_read_block(20, (char *)tmp, 1))
	{
		return TRUE;             
	}
   
	/* OK, not 1.68M - try again for 1.44M reading block #18 on first track */
	floppy->geometry.cyls = DG144_TRACKS;
	floppy->geometry.heads = DG144_HEADS;
	floppy->geometry.sectors = DG144_SECTORS;

	dprintf("floppy: Checking 1.44MB disk...\n");
	if (floppy_read_block(17, (char *)tmp, 1))
	{
		return TRUE;
	}
	/* it's not 1.44M or 1.68M - we don't support it */
	return FALSE;
}

/* read block (blockbuff is 512 byte buffer) */
BOOL floppy_read_block(int block, byte *blockbuff, unsigned long nosectors)
{
	int track=0, sector=0, head=0, track2=0, result=0, loop=0;

// The FDC can read multiple sides at once but not multiple tracks
	
	block2hts(block, &head, &track, &sector);
	block2hts(block+nosectors, &head, &track2, &sector);
	
	if(track!=track2)
	{
		for(loop=0; loop<nosectors; loop++)
			result = floppy_rw(block+loop, blockbuff+(loop*512), TRUE, 1, TRUE);
		return result;
	}
	return floppy_rw(block,blockbuff,TRUE,nosectors, TRUE);
}

/* write block (blockbuff is 512 byte buffer) */
BOOL floppy_write_block(int block,byte *blockbuff, unsigned long nosectors)
{
   return floppy_rw(block,blockbuff,FALSE, nosectors, TRUE);
}

/*
 * since reads and writes differ only by a few lines, this handles both.  This
 * function is called by read_block() and write_block()
 */
BOOL floppy_rw(int block, byte *blockbuff,BOOL read, unsigned long nosectors, BOOL firsttry)
{
	int head,track,sector,tries, copycount = 0;
	unsigned char *p_tbaddr = (char *)TBADDR;
	unsigned char *p_blockbuff = blockbuff;
   
	/* convert logical address into physical address */
	block2hts(block, &head, &track, &sector);

	/* spin up the disk */
	floppy_motoron();

	if (!read && blockbuff)
	{
		/* copy data from data buffer into track buffer */
		for(copycount=0; copycount<(nosectors*512); copycount++)
		{
			*p_tbaddr = *p_blockbuff;
			p_blockbuff++;
			p_tbaddr++;
		}
	}
   
	for (tries = 0; tries < 3; tries++)
	{
		/* check for diskchange */
		if (inportb(floppy->FDC_DIR) & 0x80)
		{
			floppy->dchange = TRUE;
			floppy_seek(1);  /* clear "disk change" status */
			floppy_recalibrate();
			floppy_motoroff();
			dprintf("FDC: Disk change detected. Trying again.\n");
	 
			return floppy_rw(block, blockbuff, read, nosectors, TRUE);
		}
		/* move head to right track */
		if (!floppy_seek(track))
		{
			floppy_motoroff();
			dprintf("FDC: Error seeking to track\n");
			return FALSE;
		}
		/* program data rate (500K/s) */
		outportb(floppy->FDC_CCR,0);
		/* send command */
		if (read)
		{
			dma_xfer(2, TBADDR, nosectors*512, FALSE);
			floppy_sendbyte(CMD_READ);
		}
		else
		{
			dma_xfer(2, TBADDR, nosectors*512, TRUE);
			floppy_sendbyte(CMD_WRITE);
		}
		floppy_sendbyte(head << 2);
		floppy_sendbyte(track);
		floppy_sendbyte(head);
		floppy_sendbyte(sector);
		floppy_sendbyte(2);               /* 512 bytes/sector */
		floppy_sendbyte(floppy->geometry.sectors);
		if (floppy->geometry.sectors == DG144_SECTORS)
			floppy_sendbyte(DG144_GAP3RW);  /* gap 3 size for 1.44M read/write */
		else
			floppy_sendbyte(DG168_GAP3RW);  /* gap 3 size for 1.68M read/write */
		floppy_sendbyte(0xff);            /* DTL = unused */
      
		/* wait for command completion */
		/* read/write don't need "sense interrupt status" */
		if (!floppy_wait(TRUE))
		{
			if (firsttry)
			{
				dprintf("Timed out, trying operation again after reset()\n");
				floppy_reset();	
				return floppy_rw(block, blockbuff, read, nosectors, FALSE);
			}
			else
			{
				dprintf("Time out...\n");
				return FALSE;
			}
		}
		if ((floppy->status[0] & 0xc0) == 0) break;   /* worked! outta here! */
		floppy_recalibrate();  /* oops, try again... */
	}
	/* stop the motor */
	floppy_motoroff();

	if (read && blockbuff)
	{
		/* copy data from track buffer into data buffer */
		p_blockbuff = blockbuff;
		p_tbaddr = (char *)TBADDR;
		for(copycount=0; copycount<(nosectors*512); copycount++)
		{
			*p_blockbuff = *p_tbaddr;
			p_blockbuff++;
			p_tbaddr++;
		}
	}
	return (tries != 3);
}

/* this formats a track, given a certain geometry */
BOOL floppy_format(byte track)
{
	int i,h,r,r_id,split;
	byte tmpbuff[256];
	unsigned char *p_tbaddr = (char *)0x8000;
	unsigned int copycount = 0;

	/* check geometry */
	if (floppy->geometry.sectors != DG144_SECTORS && floppy->geometry.sectors != DG168_SECTORS)
		return FALSE;
   
	/* spin up the disk */
	floppy_motoron();

	/* program data rate (500K/s) */
	outportb(floppy->FDC_CCR,0);

	floppy_seek(track);  /* seek to track */

	/* precalc some constants for interleave calculation */
	split = floppy->geometry.sectors / 2;
	if (floppy->geometry.sectors & 1) split++;
   
	for (h = 0;h < floppy->geometry.heads;h++)
	{
		/* for each head... */
		/* check for diskchange */
		if (inportb(floppy->FDC_DIR) & 0x80)
		{
			floppy->dchange = TRUE;
			floppy_seek(1);  /* clear "disk change" status */
			floppy_recalibrate();
			floppy_motoroff();
			return FALSE;
		}

		i = 0;   /* reset buffer index */
		for (r = 0;r < floppy->geometry.sectors;r++)
		{
			/* for each sector... */

			/* calculate 1:2 interleave (seems optimal in my system) */
			r_id = r / 2 + 1;
			if (r & 1) r_id += split;
	 
			/* add some head skew (2 sectors should be enough) */
			if (h & 1)
			{
				r_id -= 2;
				if (r_id < 1) r_id += floppy->geometry.sectors;
			}
      
			/* add some track skew (1/2 a revolution) */
			if (track & 1)
			{
				r_id -= floppy->geometry.sectors / 2;
				if (r_id < 1) r_id += floppy->geometry.sectors;
			}
			/**** interleave now calculated - sector ID is stored in r_id ****/

			/* fill in sector ID's */
			tmpbuff[i++] = track;
			tmpbuff[i++] = h;
			tmpbuff[i++] = r_id;
			tmpbuff[i++] = 2;
		}
		/* copy sector ID's to track buffer */
		for(copycount = 0; copycount<i; copycount++)
		{
			*p_tbaddr = tmpbuff[copycount];
			p_tbaddr++;
		}
		/* start dma xfer */
		dma_xfer(2, TBADDR, i, TRUE);
      
		/* prepare "format track" command */
		floppy_sendbyte(CMD_FORMAT);
		floppy_sendbyte(h << 2);
		floppy_sendbyte(2);
		floppy_sendbyte(floppy->geometry.sectors);
		if (floppy->geometry.sectors == DG144_SECTORS)      
			floppy_sendbyte(DG144_GAP3FMT);    /* gap3 size for 1.44M format */
		else
			floppy_sendbyte(DG168_GAP3FMT);    /* gap3 size for 1.68M format */
		floppy_sendbyte(0);     /* filler byte */
 
		/* wait for command to finish */
		if (!floppy_wait(FALSE))
			return FALSE;
      
		if (floppy->status[0] & 0xc0)
		{
			floppy_motoroff();
			return FALSE;
		}
	}   
	floppy_motoroff();
   
	return TRUE;
}

#define MAX_REPLIES 16
unsigned char reply_buffer[MAX_REPLIES];

int floppy_wait_til_ready(void)
{
	int counter, status;

 	for (counter=0; counter<10000; counter++)
	{
		status = inportb(floppy->FDC_MSR);
		if (status & STATUS_READY)
			return status;
	}
	return -1;
}

int floppy_result(void)
{
	int i, status;

	for (i=0; i<MAX_REPLIES-1; i++)
	{
		if ((status = floppy_wait_til_ready()) < 0)
			break;
		status &= (STATUS_DIR | STATUS_READY | STATUS_BUSY | STATUS_DMA);
		if ((status &~ STATUS_BUSY) == STATUS_READY)
			return i;
		if (status == (STATUS_DIR | STATUS_READY | STATUS_BUSY))
			reply_buffer[i] = inportb(floppy->FDC_DATA);
		else
			break;
	}
	return -1;
}

int floppy_need_more_output(void)
{
	int status;

	if ((status = floppy_wait_til_ready()) < 0)
		return -1;
	if ((status & (STATUS_READY | STATUS_DIR | STATUS_DMA)) == STATUS_READY)
		return MORE_OUTPUT;
	return floppy_result();
}

int floppy_configure(void)
{
	floppy_sendbyte(CMD_CONFIGURE);
	if (floppy_need_more_output() != MORE_OUTPUT)
		return 0;
	floppy_sendbyte(0);
	floppy_sendbyte(0x10 | (no_fifo & 0x20) | (fifo_depth & 0x0f));
	floppy_sendbyte(0);
	return 1;
}

floppy_types GetConfig(unsigned char drv)
{
	unsigned b;

	b = cmos_read_register(0x10);

	return ((!drv) ? (b >> 4 ) : (b & 0xF));
}

int detect_floppy(void)
{
	int r;

	if (GetConfig(fdc) == zero)		return FDC_UNKNOWN;
	floppy_reset();
	floppy_sendbyte(CMD_DUMPREGS);
	r = floppy_result();
	if (r <= 0x00)					return FDC_UNKNOWN;
	dprintf("floppy: FDD%d -> CMOS size = ", fdc);
	switch(GetConfig(fdc))
	{
		case d360 : dprintf("3¬\", 360 KB"); break;
		case d1200: dprintf("3¬\", 1.2 MB"); break;
		case d720 : dprintf("3¬\", 720 KB"); break;
		case d1440: dprintf("3¬\", 1.44 MB"); break;
		case d120l: dprintf("5«\", 1.2 MB"); break;
		case d144l: dprintf("5«\", 1.44 MB"); break;
		default: break;
	}
	dprintf("\n");
	if ((r == 1) && (reply_buffer[0] == 0x80))
	{
		dprintf("floppy: FDD0 is a 8272A\n");
		return FDC_8272A;
	}
	if (r != 10)
		return FDC_UNKNOWN;
	if (!floppy_configure())
	{
		dprintf("floppy: FDD0 is a 82072\n");
		return FDC_82072;
	}
	floppy_sendbyte(CMD_PERPENDICULAR);
	if (floppy_need_more_output() == MORE_OUTPUT)
		floppy_sendbyte(0);
	else
	{
		dprintf("floppy: FDD0 is a 82072A\n");
		return FDC_82072A;
	}
	floppy_sendbyte(CMD_UNLOCK);
	r = floppy_result();
	if ((r == 1) && (reply_buffer[0] == 0x80))
	{
		dprintf("floppy: FDD0 is a pre-1991 82077\n");
		return FDC_82077_ORIG;
	}
	if ((r != 1) || (reply_buffer[0] != 0x00))
	{
		dprintf("floppy: FDD init: UNLOCK: unexpected value of %d bytes\n", r);
		dprintf("floppy: FDD not found\n");
		return FDC_UNKNOWN;
	}
	floppy_sendbyte(CMD_PARTID);
	r = floppy_result();
	if (r != 1)
	{
		dprintf("floppy: FDD init: PARTID: unexpected value of %d bytes\n", r);
		return FDC_UNKNOWN;
	}
	if (reply_buffer[0] == 0x80)
	{
		dprintf("floppy: FDD0 is a post-1991 82077\n");
		return FDC_82077;
	}
	switch(reply_buffer[0] >> 5)
	{
		case 0x00:
			dprintf("floppy: FDD0 is a 82078\n");
			return FDC_82078;
		case 0x01:
		dprintf("floppy: FDD0 is a 44pin 82078\n");
			return FDC_82078;
		case 0x02:
		dprintf("floppy: FDD0 is an S82078B\n");
			return FDC_S82078B;
		case 0x03:
			dprintf("floppy: FDD0 is a National Semiconductor PC87306\n");
			return FDC_87306;
		default:
			dprintf("floppy: FDD0 init: 82078 variant with unknown PARTID=%d\n",
				reply_buffer[0] >> 5);
			return FDC_82078_UNKN;
	}
	return FDC_UNKNOWN;
}

void SetParameters(unsigned int fdc, unsigned int port)
{
	floppy->FDC_IOPORT = port;	
	floppy->FDC_DOR    = floppy->FDC_IOPORT + 0x02;
	floppy->FDC_MSR    = floppy->FDC_IOPORT + 0x04;
	floppy->FDC_DRS    = floppy->FDC_IOPORT + 0x04;
	floppy->FDC_DATA   = floppy->FDC_IOPORT + 0x05;
	floppy->FDC_DIR    = floppy->FDC_IOPORT + 0x07;
	floppy->FDC_DCR    = floppy->FDC_IOPORT + 0x07;
	floppy->FDC_CCR    = floppy->FDC_IOPORT + 0x07;	
}

int floppy_ioctl(int drive, int func, byte *buf, int sec, int sec_count)
{
	fdc = drive;
	if (func == FLOPPY_READ)
	{
		return floppy_read_block(sec, buf, sec_count);
	}
	return 0;
}

/* init driver */
int floppy_init(void)
{
	int a = 0, b = 0;

	SetParameters(0, FDC_1);
	SetParameters(1, FDC_2);
	fdc = 0;	a = (detect_floppy()) != FDC_UNKNOWN;
//	fdc = 1;	b = (detect_floppy()) != FDC_UNKNOWN;	
	if (a || b)
	{
		if (a)
		{
			fdc = 0;
			if (!floppy_get_geo()) dprintf("floppy1: error, assuming 1.44MB disk\n");
			dprintf("floppy1: C/H/S: %ld / %ld / %ld\n", floppy->geometry.cyls, floppy->geometry.heads, floppy->geometry.sectors);	
		}
		if (b)
		{
			fdc = 1;
			if (!floppy_get_geo()) dprintf("floppy2: error, assuming 1.44MB disk\n");
			dprintf("floppy2: C/H/S: %ld / %ld / %ld\n", floppy->geometry.cyls, floppy->geometry.heads, floppy->geometry.sectors);	
		}
		fdc = a ? 0 : 1;
	}
	else
		dprintf("floppy: No useable floppy drives detected...\n");

	interrupt_install(INT_HARDWARE, 6, (dword)isr_floppy);
	irq_enable(6);

	return (a | b);
}
