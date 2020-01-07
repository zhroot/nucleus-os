#include <support.h>
#include <string.h>
#include <stdio.h>
#include <video/graphic.h>
#include <video/fonts.h>

#define pokeb(S,O,V)		( *(char*) ( ( S ) * 16 + ( O ) ) = ( V ) ) 
#define pokew(S,O,V)		( *(short*) ( ( S ) * 16 + ( O ) ) = ( V ) )  
#define peekb(S,O)			(*(unsigned char *)( ((unsigned)( S ) * 16) + ( O )))  
#define	_vmemwr(DS,DO,S,N)	_dosmemputb(S, N, 16uL * (DS) + (DO))

/*****************************************************************************
VGA REGISTER DUMPS FOR VARIOUS TEXT MODES

()=to do
	40x25	(40x30)	40x50	(40x60)
	(45x25)	(45x30)	(45x50)	(45x60)
	80x25	(80x30)	80x50	(80x60)
	(90x25)	90x30	(90x50)	90x60
*****************************************************************************/

static void dump(unsigned char *regs, unsigned count)
{
	unsigned i;

	i = 0;
	printf("\t");
	for(; count != 0; count--)
	{
		printf("0x%02X,", *regs);
		i++;
		if(i >= 8)
		{
			i = 0;
			printf("\n\t");
		}
		else
			printf(" ");
		regs++;
	}
	printf("\n");
}

void dump_regs(unsigned char *regs)
{
	printf("unsigned char g_mode[] =\n");
	printf("{\n");
/* dump MISCELLANEOUS reg */
	printf("/* MISC */\n");
	printf("\t0x%02X,\n", *regs);
	regs++;
/* dump SEQUENCER regs */
	printf("/* SEQ */\n");
	dump(regs, SEQ_C);
	regs += SEQ_C;
/* dump CRTC regs */
	printf("/* CRTC */\n");
	dump(regs, CRT_C);
	regs += CRT_C;
/* dump GRAPHICS CONTROLLER regs */
	printf("/* GC */\n");
	dump(regs, GRA_C);
	regs += GRA_C;
/* dump ATTRIBUTE CONTROLLER regs */
	printf("/* AC */\n");
	dump(regs, ATT_C);
	regs += ATT_C;
	printf("};\n");
}

void read_regs(unsigned char *regs)
{
	unsigned i;

/* read MISCELLANEOUS reg */
	*regs = inportb(MIS_R);
	regs++;
/* read SEQUENCER regs */
	for(i = 0; i < SEQ_C; i++)
	{
		outportb(SEQ_I, i);
		*regs = inportb(SEQ_D);
		regs++;
	}
/* read CRTC regs */
	for(i = 0; i < CRT_C; i++)
	{
		outportb(CRT_I, i);
		*regs = inportb(CRT_D);
		regs++;
	}
/* read GRAPHICS CONTROLLER regs */
	for(i = 0; i < GRA_C; i++)
	{
		outportb(GRA_I, i);
		*regs = inportb(GRA_D);
		regs++;
	}
/* read ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < ATT_C; i++)
	{
		(void)inportb(IS1_R);
		outportb(ATT_I, i);
		*regs = inportb(ATT_R);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inportb(IS1_R);
	outportb(ATT_I, 0x20);
}

static void set_plane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
#if 0
/* set read plane */
	outportb(GRA_I, 4);
	outportb(GRA_D, p);
/* set write plane */
	outportb(SEQ_I, 2);
	outportb(SEQ_D, pmask);
#else
	outportw(GRA_I, (p << 8) | 4);
	outportw(SEQ_I, (pmask << 8) | 2);
#endif
}
/*****************************************************************************
VGA framebuffer is at A000:0000, B000:0000, or B800:0000
depending on bits in GC 6
*****************************************************************************/
static unsigned get_fb_seg(void)
{
	unsigned seg;

	outportb(GRA_I, 6);
	seg = inportb(GRA_D);
	seg >>= 2;
	seg &= 3;
	switch(seg)
	{
	case 0:
	case 1:
		seg = 0xA000;
		break;
	case 2:
		seg = 0xB000;
		break;
	case 3:
		seg = 0xB800;
		break;
	}
	return seg;
}

static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count)
{
	_vmemwr(get_fb_seg(), dst_off, src, count);
}

static void vpokeb(unsigned off, unsigned val)
{
	pokeb(get_fb_seg(), off, val);
}

static unsigned vpeekb(unsigned off)
{
	return peekb(get_fb_seg(), off);
}

/*****************************************************************************
write font to plane P4 (assuming planes are named P1, P2, P4, P8)
*****************************************************************************/
static void write_font(unsigned char *buf, unsigned font_height)
{
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned i;

/* save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
	outportb(SEQ_I, 2);
	seq2 = inportb(SEQ_D);

	outportb(SEQ_I, 4);
	seq4 = inportb(SEQ_D);
/* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
	outportb(SEQ_D, seq4 | 0x04);

	outportb(GRA_I, 4);
	gc4 = inportb(GRA_D);

	outportb(GRA_I, 5);
	gc5 = inportb(GRA_D);
/* turn off even-odd addressing */
	outportb(GRA_D, gc5 & ~0x10);

	outportb(GRA_I, 6);
	gc6 = inportb(GRA_D);
/* turn off even-odd addressing */
	outportb(GRA_D, gc6 & ~0x02);
/* write font to plane P4 */
	set_plane(2);
/* write font 0 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 0 + i * 32, buf, font_height);
		buf += font_height;
	}
#if 0
/* write font 1 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 1 + i * 32, buf, font_height);
		buf += font_height;
	}
#endif
/* restore registers */
	outportb(SEQ_I, 2);
	outportb(SEQ_D, seq2);
	outportb(SEQ_I, 4);
	outportb(SEQ_D, seq4);
	outportb(GRA_I, 4);
	outportb(GRA_D, gc4);
	outportb(GRA_I, 5);
	outportb(GRA_D, gc5);
	outportb(GRA_I, 6);
	outportb(GRA_D, gc6);
}

/*****************************************************************************
READ AND DUMP VGA REGISTER VALUES FOR CURRENT VIDEO MODE
This is where g_40x25_text[], g_80x50_text[], etc. came from :)
*****************************************************************************/
void dump_state(void)
{
	unsigned char state[VGA_NUM_REGS];

	read_regs(state);
	dump_regs(state);
}

/*****************************************************************************
SET TEXT MODES
*****************************************************************************/
char set_mode_modes(unsigned int mode)
{
	unsigned rows = 80, cols = 25, ht = 8, i = 0;

	printf("Setting mode %d...\n", mode);
	switch(mode)
	{
		case 0:
		case 1: { // text, 16 cols
				write_regs(g_40x25_text);
				cols = 40;
				rows = 25;
				ht = 16;
				} break;
		case 2:
		case 3: { // text, 16 cols
				write_regs(g_80x25_text);
				cols = 80;
				rows = 25;
				ht = 16;
				} break;
		case 4:
		case 5: { // graphics, 4 cols
					vga_set_mode(mode);
				} break;
//		case 6: { // graphics, 2 cols
//				write_regs(g_640x200x2);
//				g_wd = 640;
//				g_ht = 200;
//				g_write_pixel = write_pixel1;
//				} break;
		case 7: { // text, mono (originally)
				write_regs(g_80x25_text);
				cols = 80;
				rows = 25;
				ht = 16;
				} break;
		default: return 0xFF;
	}				
/* set font */
	if(ht >= 16)
		write_font(g_8x16_font, 16);
	else
		write_font(g_8x8_font, 8);
/* tell the BIOS what we've done, so BIOS text output works OK */
	pokew(0x40, 0x4A, cols);	/* columns on screen */
	pokew(0x40, 0x4C, cols * rows * 2); /* framebuffer size */
	pokew(0x40, 0x50, 0);		/* cursor pos'n */
	pokeb(0x40, 0x60, ht - 1);	/* cursor shape */
	pokeb(0x40, 0x61, ht - 2);
	pokeb(0x40, 0x84, rows - 1);	/* rows on screen - 1 */
	pokeb(0x40, 0x85, ht);		/* char height */
/* set white-on-black attributes for all text */
	for(i = 0; i < cols * rows; i++)
		pokeb(0xB800, i * 2 + 1, 7);
	return 0;
}
/*****************************************************************************
DEMO GRAPHICS MODES
*****************************************************************************/
static void demo_graphics(void)
{
	printf("Screen-clear in 16-color mode will be VERY SLOW\n"
		"Press a key to continue\n");
/* 4-color */
	write_regs(g_320x200x4);
	g_wd = 320;
	g_ht = 200;
	g_write_pixel = write_pixel2;
	draw_x();
/* 16-color */
	write_regs(g_640x480x16);
	g_wd = 640;
	g_ht = 480;
	g_write_pixel = write_pixel4p;
	draw_x();
/* 256-color */
	write_regs(g_320x200x256);
	g_wd = 320;
	g_ht = 200;
	g_write_pixel = write_pixel8;
	draw_x();
/* 256-color Mode-X */
	write_regs(g_320x200x256_modex);
	g_wd = 320;
	g_ht = 200;
	g_write_pixel = write_pixel8x;
	draw_x();
/* go back to 80x25 text mode */
	set_mode_modes(7);
}

static unsigned char reverse_bits(unsigned char arg)
{
	unsigned char ret_val = 0;

	if(arg & 0x01)
		ret_val |= 0x80;
	if(arg & 0x02)
		ret_val |= 0x40;
	if(arg & 0x04)
		ret_val |= 0x20;
	if(arg & 0x08)
		ret_val |= 0x10;
	if(arg & 0x10)
		ret_val |= 0x08;
	if(arg & 0x20)
		ret_val |= 0x04;
	if(arg & 0x40)
		ret_val |= 0x02;
	if(arg & 0x80)
		ret_val |= 0x01;
	return ret_val;
}

/*****************************************************************************
512-CHARACTER FONT
*****************************************************************************/
static void font512(void)
{
/* Turbo C++ 1.0 seems to 'lose' any data declared 'static const' */
	/*static*/ const char msg1[] = "!txet sdrawkcaB";
	/*static*/ const char msg2[] = "?rorrim a toG";
/**/
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned font_height, i, j;

/* start in 80x25 text mode */
	set_mode_modes(7);
/* code pasted in from write_font():
save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
	outportb(SEQ_I, 2);
	seq2 = inportb(SEQ_D);

	outportb(SEQ_I, 4);
	seq4 = inportb(SEQ_D);
/* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
	outportb(SEQ_D, seq4 | 0x04);

	outportb(GRA_I, 4);
	gc4 = inportb(GRA_D);

	outportb(GRA_I, 5);
	gc5 = inportb(GRA_D);
/* turn off even-odd addressing */
	outportb(GRA_D, gc5 & ~0x10);

	outportb(GRA_I, 6);
	gc6 = inportb(GRA_D);
/* turn off even-odd addressing */
	outportb(GRA_D, gc6 & ~0x02);
/* write font to plane P4 */
	set_plane(2);
/* this is different from write_font():
use font 1 instead of font 0, and use it for BACKWARD text */
	font_height = 16;
	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < font_height; j++)
		{
			vpokeb(16384u * 1 + 32 * i + j,
				reverse_bits(
					g_8x16_font[font_height * i + j]));
		}
	}
/* restore registers */
	outportb(SEQ_I, 2);
	outportb(SEQ_D, seq2);
	outportb(SEQ_I, 4);
	outportb(SEQ_D, seq4);
	outportb(GRA_I, 4);
	outportb(GRA_D, gc4);
	outportb(GRA_I, 5);
	outportb(GRA_D, gc5);
	outportb(GRA_I, 6);
	outportb(GRA_D, gc6);
/* now: sacrifice attribute bit b3 (foreground intense color)
use it to select characters 256-511 in the second font */
	outportb(SEQ_I, 3);
	outportb(SEQ_D, 4);
/* xxx - maybe re-program 16-color palette here
so attribute bit b3 is no longer used for 'intense' */
	for(i = 0; i < sizeof(msg1); i++)
	{
		vpokeb((80 * 8  + 40 + i) * 2 + 0, msg1[i]);
/* set attribute bit b3 for backward font */
		vpokeb((80 * 8  + 40 + i) * 2 + 1, 0x0F);
	}
	for(i = 0; i < sizeof(msg2); i++)
	{
		vpokeb((80 * 16 + 40 + i) * 2 + 0, msg2[i]);
		vpokeb((80 * 16 + 40 + i) * 2 + 1, 0x0F);
	}
}
