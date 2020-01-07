#ifndef _IDE_H_
#define _IDE_H_

struct IDE_ADDR
{
	word port;
	byte out;
};

static const struct IDE_ADDR ide_addr[8] =
{
	{0x1F0, 0xA0}, {0x1F0, 0xB0},
	{0x170, 0xA0}, {0x170, 0xB0},
	{0x1E8, 0xA0}, {0x1E8, 0xB0},
	{0x168, 0xA0}, {0x168, 0xB0}
};

typedef struct ide_data
{
	word config, cyls, reserved2, heads, track_bytes, sector_bytes;
	word sectors, vendor0, vendor1, vendor2;
	char serial[20];
	word buf_type, buf_size, ecc_bytes;
	char rev[8];
	char model[40];
	byte max_multsect, vendor3;
	word dword_io;
	byte vendor4, capability;
	word reserved50;
	word vendor5, tPIO, vendor6, tDMA;
	word field_valid, cur_cyls, cur_heads;
	word cur_sectors, cur_capacity0, cur_capacity1;
	byte multsect, multsect_valid;
	word lba_capacity;
	word dma_1word, dma_mword, eide_pio_modes, eide_dma_min;
	word eide_dma_time, eide_pio, ide_pio_iordy;
	word word69_81[13];
	word command_sets;
	word word83_87[5];
	word dma_ultra;
	word words[40];
	word security;
	word reserved[127];
} ide_data;

struct IDE_DRIVE
{
	word port;
	byte out;
	word flags;
	char * model, * serial, * rev;
	ide_data data;
};

#define	ATA_CMD_RD		0x20					/* read one sector */
#define	ATA_CMD_WR		0x30					/* write one sector */
#define	ATA_CMD_PKT		0xA0					/* ATAPI packet cmd */
#define	ATA_CMD_PID		0xA1					/* ATAPI identify */
#define	ATA_CMD_RDMUL	0xC4					/* read multiple sectors */
#define	ATA_CMD_WRMUL	0xC5					/* write multiple sectors */
#define	ATA_CMD_ID		0xEC					/* ATA identify */

/* ATA drive flags */
#define	ATA_FLG_ATAPI	0x0001					/* ATAPI drive */
#define ATA_FLG_LBA		0x0002					/* LBA-capable */
#define ATA_FLG_DMA		0x0004					/* DMA-capable */

/* ATA/ATAPI drive register file */
#define	ATA_REG_DATA	0					/* data (16-bit) */
#define	ATA_REG_FEAT	1					/* write: feature reg */
#define	ATA_REG_ERR		ATA_REG_FEAT		/* read: error */
#define	ATA_REG_CNT		2					/* ATA: sector count */
#define	ATA_REG_REASON	ATA_REG_CNT			/* ATAPI: interrupt reason */
#define	ATA_REG_SECT	3					/* sector */
#define	ATA_REG_LOCYL	4					/* ATA: LSB of cylinder */
#define	ATA_REG_LOCNT	ATA_REG_LOCYL		/* ATAPI: LSB of transfer count */
#define	ATA_REG_HICYL	5					/* ATA: MSB of cylinder */
#define	ATA_REG_HICNT	ATA_REG_HICYL		/* ATAPI: MSB of transfer count */
#define	ATA_REG_DRVHD	6					/* drive select; head */
#define	ATA_REG_CMD		7					/* write: drive command */
#define	ATA_REG_STAT	7					/* read: status and error flags */
#define	ATA_REG_SLCT	0x206				/* write: device control */
#define	ATA_REG_ALTST	0x206				/* read: alternate status/error */

int ide_init(void);

#endif
