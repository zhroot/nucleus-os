#ifndef _FLOPPY_H
#define _FLOPPY_H

#include <datatypes.h>
#include <drivers/block/disk.h>

#define FLOPPY_READ		0x01
#define FLOPPY_WRITE	0x02

/* datatypes */
typedef enum {zero, d360, d1200, d720, d1440, d120l, d144l} floppy_types;

typedef struct floppy_t
{
	floppy_types type;
	word port;
	BOOL dchange;
	BOOL motor;
	byte track;
	byte status[7];
	byte statsz;
	byte sr0;
	unsigned int FDC_IOPORT, FDC_MSR, FDC_DRS, FDC_DATA;
	unsigned int FDC_DOR, FDC_DIR, FDC_DCR, FDC_CCR;
	struct CHS geometry;
} floppy_t;

#define FDC_1			0x3F0
#define FDC_2			0x370

#define FDC_NONE	0x00
#define FDC_UNKNOWN	0x10
#define FDC_8272A	0x20	// Intel 8272a, NEC 765
#define FDC_765ED	0x30	// Non-Intel 1MB-compatible FDC, can't detect
#define FDC_82072	0x40	// Intel 82072; 8272a + FIFO + DUMPREGS
#define FDC_82072A	0x45	// 82072A (on Sparcs)
#define FDC_82077_ORIG	0x51	// Original version of 82077AA, sans LOCK
#define FDC_82077	0x52	// 82077AA-1
#define FDC_82078_UNKN	0x5f	// Unknown 82078 variant
#define FDC_82078	0x60	// 44pin 82078 or 64pin 82078SL
#define FDC_82078_1	0x61	// 82078-1 (2Mbps fdc)
#define FDC_S82078B	0x62	// S82078B (first seen on Adaptec AVA-2825 VLB
				// SCSI/EIDE/Floppy controller)
#define FDC_87306	0x63	// National Semiconductor PC 87306
#define MORE_OUTPUT	-2


#define fifo_depth	0x0A
#define no_fifo		0x00

/* drive geometries */
#define DG144_HEADS       2     /* heads per drive (1.44M) */
#define DG144_TRACKS     80     /* number of tracks (1.44M) */
#define DG144_SECTORS    18     /* sectors per track (1.44M) */
#define DG144_GAP3FMT  0x54     /* gap3 while formatting (1.44M) */
#define DG144_GAP3RW   0x1b     /* gap3 while reading/writing (1.44M) */

#define DG168_HEADS       2     /* heads per drive (1.68M) */
#define DG168_TRACKS     80     /* number of tracks (1.68M) */
#define DG168_SECTORS    21     /* sectors per track (1.68M) */
#define DG168_GAP3FMT  0x0c     /* gap3 while formatting (1.68M) */
#define DG168_GAP3RW   0x1c     /* gap3 while reading/writing (1.68M) */


/* command bytes (these are 765 commands + options such as MFM, etc) */
#define CMD_SPECIFY		(0x03)  /* specify drive timings */
#define CMD_WRITE		(0xc5)  /* write data (+ MT,MFM) */
#define CMD_READ		(0xe6)  /* read data (+ MT,MFM,SK) */
#define CMD_RECAL		(0x07)  /* recalibrate */
#define CMD_SENSEI		(0x08)  /* sense interrupt status */
#define CMD_FORMAT		(0x4d)  /* format track (+ MFM) */
#define CMD_SEEK		(0x0f)  /* seek track */
#define CMD_VERSION		(0x10)  /* FDC version */
#define CMD_DUMPREGS	(0x0E)
#define CMD_PERPENDICULAR	(0x12)
#define CMD_CONFIGURE	(0x13)
#define CMD_UNLOCK		(0x14)
#define CMD_PARTID		(0x18)

#define STATUS_BUSYMASK		0x0F
#define STATUS_BUSY			0x10
#define STATUS_DMA			0x20
#define STATUS_DIR			0x40
#define STATUS_READY		0x80

/* function prototypes */

int floppy_init();
/*void deinit(void); */

void floppy_update(void);
void test_floppy(void);
int floppy_ioctl(int drive, int func, byte *buf, int sec, int sec_count);

#endif /* FDC_H */
