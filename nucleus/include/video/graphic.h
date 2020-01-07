#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

typedef struct 
{
	unsigned int (*DetectChip)(void);
	char * (*DetectName)(void);
	unsigned int (*DetectMemory)(void);
	char (*SetMode)(unsigned int mode);
	unsigned int (*GetMode)(void);
	void (*SetBank)(unsigned int bank);
	void (*SetReadBank)(unsigned int bank);
	void (*WritePixel)(unsigned int x, unsigned int y, unsigned long col);
	unsigned long (*ReadPixel)(unsigned int x, unsigned int y);
	void (*WriteChar)(unsigned int x, unsigned int y, unsigned char b);
	unsigned int (*GetX)(void);
	unsigned int (*GetY)(void);
	unsigned int (*GetMaxX)(void);
	unsigned int (*GetMaxY)(void);
	unsigned long (*GetMaxCol)(void);	
	void (*GotoXY)(unsigned int x, unsigned int y);
	void (*Scroll)(long lines);		// < 0 scroll up, > 0 scroll down
	void (*ClrScr)(void);			// have to be driver dependent
} GraphicDriver;

extern GraphicDriver vga_driver;	// default driver

struct _BIOS_MODES
{
	unsigned char *video_ptr;
	unsigned int max_x, max_y;
	unsigned int max_cols;
	unsigned char font_width, font_height;
	unsigned char pages;
	char graphic;
} BIOS_MODES;

static struct _BIOS_MODES bios_modes[0x13+1] =
{
	{ (char *)0xB8000,   40,   25,  16,  9, 16, 8, 0 },	// 0x00
	{ (char *)0xB8000,   40,   25,  16,  9, 16, 8, 0 },	// 0x01
	{ (char *)0xB8000,   80,   25,  16,  9, 16, 8, 0 },	// 0x02
	{ (char *)0xB8000,   80,   25,  16,  9, 16, 8, 0 },	// 0x03
	{ (char *)0xB8000,  320,  200,   4,  8,  8, 1, 1 },	// 0x04
	{ (char *)0xB8000,  320,  200,   4,  8,  8, 1, 1 },	// 0x05
	{ (char *)0xB8000,  640,  200,   2,  8,  8, 1, 1 },	// 0x06
	{ (char *)0xB8000,   80,   25,  16,  9, 16, 8, 0 },	// 0x07
	{               0,    0,    0,   0,  0,  0 },	// 0x08
	{               0,    0,    0,   0,  0,  0 },	// 0x09
	{               0,    0,    0,   0,  0,  0 },	// 0x0A
	{               0,    0,    0,   0,  0,  0 },	// 0x0B
	{               0,    0,    0,   0,  0,  0 },	// 0x0C
	{ (char *)0xA0000,  320,  200,  16,  8,  8, 8, 1 },	// 0x0D
	{ (char *)0xA0000,  640,  200,  16,  8,  8, 4, 1 },	// 0x0E
	{ (char *)0xB8000,   40,   25,  16,  8, 14, 8, 0 },	// 0x0F
	{ (char *)0xA0000,  640,  350,  16,  8, 14, 1, 1 },	// 0x10
	{ (char *)0xA0000,  640,  480,   2,  8, 16, 1, 1 },	// 0x11
	{ (char *)0xA0000,  640,  480,  16,  8, 16, 1, 1 },	// 0x12
	{ (char *)0xB8000,  320,  200, 256,  8,  8, 1, 1 }	// 0x13 ModeX
};

/* VGA index register ports */
#define	CRT_IC	0x3D4		/* CRT Controller Index - color emulation */
#define	CRT_IM	0x3B4		/* CRT Controller Index - mono emulation */
#define	ATT_I	0x3C0		/* Attribute Controller Index & Data Write Register */
#define	GRA_I	0x3CE		/* Graphics Controller Index */
#define	SEQ_I	0x3C4		/* Sequencer Index */
#define	PEL_IW	0x3C8		/* PEL Write Index */
#define	PEL_IR	0x3C7		/* PEL Read Index */

/* VGA data register ports */
#define	CRT_DC	0x3D5		/* CRT Controller Data Register - color emulation */
#define	CRT_DM	0x3B5		/* CRT Controller Data Register - mono emulation */
#define	ATT_W	0x3C0		/* Attribute Controller Data Read Register */
#define	ATT_R	0x3C1		/* Attribute Controller Data Read Register */
#define	GRA_D	0x3CF		/* Graphics Controller Data Register */
#define	SEQ_D	0x3C5		/* Sequencer Data Register */
#define	MIS_R	0x3CC		/* Misc Output Read Register */
#define	MIS_W	0x3C2		/* Misc Output Write Register */
#define	IS1_RC	0x3DA		/* Input Status Register 1 - color emulation */
#define	IS1_RM	0x3BA		/* Input Status Register 1 - mono emulation */
#define	PEL_D	0x3C9		/* PEL Data Register */
#define	PEL_MSK	0x3C6		/* PEL mask register */

/* EGA-specific registers */

#define	GRA_E0	0x3CC		/* Graphics enable processor 0 */
#define	GRA_E1	0x3CA		/* Graphics enable processor 1 */

/* standard VGA indexes max counts */
#define	CRT_C	25		/* 25 CRT Controller Registers */
#define	ATT_C	21		/* 21 Attribute Controller Registers */
#define	GRA_C	9		/* 9  Graphics Controller Registers */
#define	SEQ_C	5		/* 5  Sequencer Registers */
#define	MIS_C	1		/* 1  Misc Output Register */
#define	EXT_C	11		/* 11 SVGA Extended Registers */

#define	VGA_NUM_REGS	(1 + SEQ_C + CRT_C + GRA_C + ATT_C)
#define	TOTAL_REGS		VGA_NUM_REGS + EXT_C

/* VGA registers saving indexes */
#define CRT		0			/* CRT Controller Registers start */
#define ATT		(CRT+CRT_C)	/* Attribute Controller Registers start */
#define GRA		(ATT+ATT_C)	/* Graphics Controller Registers start */
#define SEQ		(GRA+GRA_C)	/* Sequencer Registers */
#define MIS		(SEQ+SEQ_C)	/* General Registers */
#define EXT		(MIS+MIS_C)	/* SVGA Extended Registers */ 

/* VGA index register ports */
extern unsigned int CRT_I;	/* CRT Controller Index (mono: 0x3B4) */
#define	ATT_IW	0x3C0	/* Attribute Controller Index & Data Write Register */
#define	GRA_I	0x3CE	/* Graphics Controller Index */
#define	SEQ_I	0x3C4	/* Sequencer Index */
#define	PEL_IW	0x3C8	/* PEL Write Index */

/* VGA data register ports */
extern unsigned int CRT_D;   /* CRT Controller Data Register (mono: 0x3B5) */
#define	ATT_R  0x3C1   /* Attribute Controller Data Read Register */
#define	GRA_D  0x3CF   /* Graphics Controller Data Register */
#define	SEQ_D  0x3C5   /* Sequencer Data Register */
#define	MIS_R  0x3CC   /* Misc Output Read Register */
#define	MIS_W  0x3C2   /* Misc Output Write Register */
#define	PEL_D  0x3C9   /* PEL Data Register */

#define GR_UNKNOWN		0
#define GR_MONO			(GR_UNKNOWN+1)
#define GR_HERC			(GR_MONO+1)
#define GR_CGA			(GR_HERC+1)
#define	GR_EGA			(GR_CGA+1)
#define	GR_VGA			(GR_EGA+1)
#define	GR_SiS			(GR_VGA+1)
#define	GR_S3			(GR_SiS+1)
#define	GR_Nvidia		(GR_S3+1)
#define GR_Mach32		(GR_Nvidia+1)
#define GR_Mach64		(GR_Mach32+1)
#define GR_3Dfx			(GR_Mach64+1)
#define GR_3DLabs		(GR_3Dfx+1)
#define GR_Ark			(GR_3DLabs+1)
#define GR_Ahead		(GR_Ark+1)
#define GR_Acer 		(GR_Ahead+1)
#define GR_Ati			(GR_Acer+1)
#define GR_ALG			(GR_Ati+1)
#define GR_Alliance		(GR_ALG+1)
#define GR_Cirrus		(GR_Alliance+1)
#define GR_Compaq		(GR_Cirrus+1)
#define GR_Cyrix		(GR_Compaq+1)
#define GR_Genoa		(GR_Cyrix+1)
#define GR_Video7		(GR_Genoa+1)
#define GR_UMC			(GR_Video7+1)
#define GR_HMC			(GR_UMC+1)
#define GR_HiQ			(GR_HMC+1)
#define GR_Trident		(GR_HiQ+1)
#define GR_Imagine		(GR_Trident+1)
#define GR_SMOS			(GR_Imagine+1)
#define GR_Rendition	(GR_SMOS+1)
#define GR_TSENG		(GR_Rendition+1)
#define GR_Sierra		(GR_TSENG+1)
#define GR_Sigma		(GR_Sierra+1)
#define GR_OAK			(GR_Sigma+1)
#define GR_Paradise		(GR_OAK+1)
#define GR_OPTi			(GR_Paradise+1)
#define GR_Realtek		(GR_OPTi+1)
#define GR_P2000		(GR_Realtek+1)
#define GR_MXIC			(GR_P2000+1)
#define GR_NCR			(GR_MXIC+1)
#define GR_NeoMagic		(GR_NCR+1)

extern unsigned int graphic_card;
extern unsigned int IS1_R;   /* CRT Controller Data Register (mono: 0x3B5) */

#define CL_BLACK				0x00
#define CL_BLUE					0x01
#define CL_GREEN				0x02
#define CL_CYAN					0x03
#define CL_RED					0x04
#define CL_MAGENTA				0x05
#define CL_BROWN				0x06	
#define CL_LIGHT_GRAY			0x07
#define CL_DARK_GRAY			0x08
#define CL_LIGHT_BLUE			0x09
#define CL_LIGHT_GREEN			0x0A
#define CL_LIGHT_CYAN			0x0B
#define CL_LIGHT_RED			0x0C
#define CL_LIGHT_MAGENTA		0x0D
#define CL_YELLOW				0x0E
#define CL_WHITE				0x0F
#define CL_BACK_BLACK			0x00
#define CL_BACK_BLUE			0x10
#define CL_BACK_GREEN			0x20
#define CL_BACK_CYAN			0x30
#define CL_BACK_RED				0x40
#define CL_BACK_MAGENTA			0x50
#define CL_BACK_BROWN			0x60
#define CL_BACK_LIGHT_GRAY		0x70
#define CL_BACK_DARK_GRAY		0x80
#define CL_BACK_LIGHT_BLUE		0x90
#define CL_BACK_LIGHT_GREEN		0xA0
#define CL_BACK_LIGHT_CYAN		0xB0
#define CL_BACK_LIGHT_RED		0xC0
#define CL_BACK_LIGHT_MAGENTA 	0xD0
#define CL_BACK_YELLOW			0xE0
#define CL_BACK_WHITE			0xF0

char graphic_init(void);
void getbios(char * tmp, unsigned int start, unsigned int len);
char set_mode(unsigned int mode);
void write_regs(unsigned char *regs);
void graphic_info(void);

void putch(unsigned int x, unsigned int y, unsigned char b);
unsigned int wherex(void);
unsigned int wherey(void);
void gotoxy(unsigned int x, unsigned int y);
void clrscr(void);
unsigned int max_x(void);
unsigned int max_y(void);
void scroll(unsigned int lines);

extern void port_out(int value, int port);
extern void port_outw(int value, int port);
extern void port_outl(int value, int port);
extern int port_in(int port);
extern int port_inw(int port);
extern int port_inl(int port);
extern void graphic_delay(void);

#endif
