#include <support.h>
#include <video/graphic.h>
#include <video/modes.h>
#include <video/fonts.h>
#include <video/palette.h>
#include <drivers/mem/mem.h>
#include <drivers/timer.h>

static unsigned char * font_buf = g_8x8_font;
static unsigned int vga_card = GR_VGA;
static unsigned int cursor_position = 0;
static unsigned int current_mode = 0x03;	// text mode

#define pokeb(A,V)		( *(char*) ( A ) = ( V ) ) 
#define pokew(A,V)		( *(short*) ( A ) = ( V ) )  

static struct Screen
{
	unsigned int x, y;
	unsigned color_fg, color_bg;
} screen = { 1, 1, CL_WHITE, CL_BLACK };

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
		case 1:	seg = 0xA0000;	break;
		case 2:	seg = 0xB0000;	break;
		case 3:	seg = 0xB8000;	break;
	}
	return seg;
}

#define	_vmemwr(A,S,N)	_dosmemputb(S, N, A)

static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count)
{
	_vmemwr(get_fb_seg() + dst_off, src, count);
}

static void vga_putpixel(unsigned int x, unsigned int y, unsigned color);
static void vga_gotoxy ( unsigned int x, unsigned int y );
static unsigned int vga_wherex ( void );
static unsigned int vga_wherey ( void );
static void vga_scroll(long lines);

static unsigned int getcuraddr ( void )
{
	byte b1, b2 ;	

	outportb ( CRT_I, 14 ) ;
	b1 = inportb ( CRT_D ) ;
	outportb ( CRT_I, 15 ) ;
	b2 = inportb ( CRT_D ) ;
	
	return ( ((unsigned int)b1 << 8) | ((unsigned int)b2) ) ;
}

static void setcuraddr ( unsigned int addr )
{
	outportb ( CRT_I, 14 ) ;
	outportb ( CRT_D, (addr & 0x0FF00) >> 8 ) ;
	outportb ( CRT_I, 15 ) ;
	outportb ( CRT_D, addr & 0x0FF ) ;
	
	cursor_position = addr * 2;
}

static void text_show_cursor(void)
{
	outportb(CRT_I, 0x0A);
	outportb(CRT_D, inportb(CRT_D) | 0x20);
}

static void text_hide_cursor(void)
{
	outportb(CRT_I, 0x0A);
	outportb(CRT_D, inportb(CRT_D) & ~0x20);
}

static void vga_putch(unsigned int xx, unsigned int yy, unsigned char data)
{
	unsigned int x, y, i, j;
	unsigned char ch;
	
	vga_gotoxy(xx, yy);
	x = vga_wherex();
	y = vga_wherey();
	if (bios_modes[current_mode].graphic)
	{
		if (data == '\r')
		{
			vga_gotoxy (1, y);
			return;
		}
		if (data == '\n')
		{
			x = 1;
			y++;
			if (y > bios_modes[current_mode].max_y / bios_modes[current_mode].font_height)
			{
				vga_scroll(1);
				y = bios_modes[current_mode].max_y / bios_modes[current_mode].font_height;
			}
			vga_gotoxy(x, y);
		}
		else
		{
			for (i=0; i < bios_modes[current_mode].font_height; i++)
			{
				ch = font_buf[data * bios_modes[current_mode].font_height + i];

				for (j=0; j < bios_modes[current_mode].font_width; j++)	
				{
					if (ch & 0x80)
						vga_putpixel(screen.x + j, screen.y + i, screen.color_fg);
					else
						vga_putpixel(screen.x + j, screen.y + i, screen.color_bg);
					ch <<= 1;
				}
			}
			x++;
			if (x > bios_modes[current_mode].max_x / bios_modes[current_mode].font_width)
			{				
				x = 1;
				y++;
		
				if (y > bios_modes[current_mode].max_y / bios_modes[current_mode].font_height)
				{
					//vga_scroll(1);
					y = bios_modes[current_mode].max_y / bios_modes[current_mode].font_height;
				}
			}
			vga_gotoxy(x, y);
		}
	}
	else
	{
		if (data == '\r')
		{
			vga_gotoxy (1, y);
			return;
		}

		if (data == '\n')
		{
			x = 1;		y++;
			if (y > bios_modes[current_mode].max_y)
			{
				vga_scroll(1);
				y = bios_modes[current_mode].max_y;
			}
			vga_gotoxy(x, y);
		}
		else
		{				
			bios_modes[current_mode].video_ptr[cursor_position] = data;
			bios_modes[current_mode].video_ptr[cursor_position + 1] = screen.color_fg;
			x++;
			
			if (x > bios_modes[current_mode].max_x)
			{
				x = 1;		y++;			
				if (y > bios_modes[current_mode].max_y)
				{
					vga_scroll(1);
					y = bios_modes[current_mode].max_y;
				}
			}
			vga_gotoxy(x, y);
		}
	}
}

static unsigned int vga_wherex ( void )
{
	if (bios_modes[current_mode].graphic)
	{
		return (screen.x / bios_modes[current_mode].font_width) + 1;
	}
	return ( getcuraddr() % bios_modes[current_mode].max_x ) + 1;
}

static unsigned int vga_wherey ( void )
{
	if (bios_modes[current_mode].graphic)
	{
		return (screen.y / bios_modes[current_mode].font_height) + 1;
	}
	return ( getcuraddr() / bios_modes[current_mode].max_x ) + 1;
}

static unsigned int vga_max_x(void)
{
	return bios_modes[current_mode].max_x;
}
	
static unsigned int vga_max_y(void)
{
	return bios_modes[current_mode].max_y;
}

static unsigned long vga_max_col(void)
{
	return bios_modes[current_mode].max_cols;
}

static void vga_gotoxy ( unsigned int x, unsigned int y )
{	
	if (x > 0 && x <= bios_modes[current_mode].max_x && y > 0 && y <= bios_modes[current_mode].max_y)
	{
		x--;	y--;
		if (!bios_modes[current_mode].graphic)
		{
			screen.x = x;
			screen.y = y;
			setcuraddr ((y * bios_modes[current_mode].max_x) + x);
		}
		else
		{
			screen.x = x * bios_modes[current_mode].font_width;
			screen.y = y * bios_modes[current_mode].font_height;
		}
	}
}

// fixed: scroll didn't draw more blank lines if lines > 1
static void vga_scroll(long lines)
{
	unsigned tmp1, tmp2, div;
	
	if (lines > 0)
	{
		if (bios_modes[current_mode].graphic)
		{
			switch(current_mode)
			{
				case 0x04:
				case 0x05:	
				case 0x13:	div = 4; break;
				case 0x06:	
				case 0x0D:
				case 0x0E:
				case 0x0F:
				case 0x10:
				case 0x11:
				case 0x12:	div = 8; break;
				default: div = 8;
			}			
			
			tmp1 = lines * bios_modes[current_mode].max_x * (bios_modes[current_mode].font_height);			
			tmp1 /= div;
			tmp2 = (bios_modes[current_mode].max_x * bios_modes[current_mode].max_y) - tmp1;
			tmp2 /= div;
			
			// Move all the text on the screen up, discard top line(s)
			memmove((char *)bios_modes[current_mode].video_ptr,
				(char *)bios_modes[current_mode].video_ptr + tmp1, tmp2);
				
			tmp2 = (bios_modes[current_mode].max_x * bios_modes[current_mode].max_y) -
				((bios_modes[current_mode].font_height) * lines);
			tmp2 /= div;
			
			// Draw a line of spaces at the bottom			
			memset((char *)bios_modes[current_mode].video_ptr + tmp2, screen.color_bg, tmp1);
			vga_gotoxy(1, bios_modes[current_mode].max_y - bios_modes[current_mode].font_height - 1);
		}
		else
		{
			tmp1 = lines * bios_modes[current_mode].max_x * 2;
			tmp2 = (bios_modes[current_mode].max_x * bios_modes[current_mode].max_y * 2) - tmp1;
			// Move all the text on the screen up, discard top line(s)
			memmove((char *)bios_modes[current_mode].video_ptr,
				(char *)bios_modes[current_mode].video_ptr + tmp1, tmp2);
				
			tmp1 = bios_modes[current_mode].max_x * (bios_modes[current_mode].max_y-1) * 2; 
			// Draw a line of spaces at the bottom
			memsetw((char *)bios_modes[current_mode].video_ptr + tmp1,
				(((screen.color_bg << 4) | screen.color_fg) << 8) | ' ', lines * bios_modes[current_mode].max_x);
			vga_gotoxy(1, bios_modes[current_mode].max_y);
		}
	}
}

static void vga_clrscr(void)
{
	if (!bios_modes[current_mode].graphic)
	{
		memsetw((char *)bios_modes[current_mode].video_ptr, MAKEWORD(' ', screen.color_bg),
			(bios_modes[current_mode].max_x * bios_modes[current_mode].max_y));
		setcuraddr(0);
	}
	else
//	switch(current_mode)
	{
//		case 0x13:
//			{
				outportb(SEQ_I, 2);		// index map register
				outportb(SEQ_D, 0x0F);	// allow write access on all levels
				memset((char *)bios_modes[current_mode].video_ptr, CL_BLACK, 65535);
				screen.x = 1;
				screen.y = 1;
//			} break;
	}
}

// only for vga
// mm = 0	EGA	a0000 -bffff
// mm = 1	EGA	a0000 -affff
// mm = 2	MDA	b0000 -b7fff
// mm = 3	CGA	b8000 -b8fff
void vga_mmap(int mm)
{
	mm &= 3;
	outportb(GRA_I, 6);
	if (vga_card == GR_EGA)
		outportb(GRA_D, (0x0E & 3) | (mm << 3));	// GRA_D not readable
	else
		outportb(GRA_D, (inportb(GRA_D) & 3) | (mm << 2));	// bits 2, 3
}

static void vga_putpixel(unsigned int x, unsigned int y, unsigned color)
{
	unsigned int offset, mask, col_mask[4] = {0x00, 0x55, 0xAA, 0xFF};
	unsigned char tmp;
	
	switch(current_mode)
	{
		case 0x04:
		case 0x05:
			{
				offset = (y * bios_modes[current_mode].max_x) +  (x / 4) + (8152 * (y & 1));
				mask = 0xC0 >> ((x & 3) * 2);
				tmp = bios_modes[current_mode].video_ptr[offset];
				tmp = (tmp & (mask & 0xFF)) | (col_mask[color] & mask);
				bios_modes[current_mode].video_ptr[offset] = tmp;
			} break;
		case 0x06:
			{
				offset = ((y * bios_modes[current_mode].max_x) + x) / 8 + (8152 * (y & 1));
				mask = 0x80 >> (x & 7);				
				if (color)
					bios_modes[current_mode].video_ptr[offset] |= mask;
				else
					bios_modes[current_mode].video_ptr[offset] &= (mask & 0xFF);
			} break;
		case 0x11:
			{
				offset = ((y * bios_modes[current_mode].max_x) + x) / 8;
				mask = 0x80 >> (x & 7);
				if (color)
					bios_modes[current_mode].video_ptr[offset] |= mask;
				else
					bios_modes[current_mode].video_ptr[offset] &= (mask & 0xFF);
			} break;
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x12:
			{
				if (current_mode == 0x0F)
				{
					if (color > 1)	// if bit 0, set bit 2
						color += 2;
				}
				
				offset = ((y * bios_modes[current_mode].max_x) + x) / 8;
				mask = 0x80 >> (x & 7);
				// load S/R with color
				outportb(GRA_I, 0);		outportb(GRA_D, color);
				// open memory for S/R
				outportb(GRA_I, 1);		outportb(GRA_D, 0x0F);
				// load bitmask
				outportb(GRA_I, 8);
				outportb(GRA_D, mask);
				// write color
				bios_modes[current_mode].video_ptr[offset]--;
				// reset
				outportb(GRA_I, 1);		outportb(GRA_D, 0);
				outportb(GRA_I, 8);		outportb(GRA_D, 0xFF);
			} break;
		case 0x13:
			{
				offset = ((y * bios_modes[current_mode].max_x) + x) / 4;
				outportb(SEQ_I, 2);				// index map register
				outportb(SEQ_D, 1 << (x & 3));	// select memory level
				bios_modes[current_mode].video_ptr[offset] = color;
			} break;
	}
}

static unsigned vga_getpixel(unsigned int x, unsigned int y)
{
	switch(current_mode)
	{
		case 0x13:
			{
				unsigned int offset;
				
				offset = (y * bios_modes[current_mode].max_x * 2) + (x / 4);
				outportb(GRA_I, 4);			// index map register select
				outportb(GRA_D, x & 3);		// select memory level
				return bios_modes[current_mode].video_ptr[offset];
			} break;
	}
	return 0;
}

static char vga_setmode(unsigned int mode)
{
	switch(mode)
	{
		case 0x00:		// text, 40x25, 16 cols
			{
				write_regs(g_mode_00);
				setpalette16();
			} break;
		case 0x01:		// text, 40x25, 16 cols
			{
				write_regs(g_mode_01);
				setpalette16();
			} break;
		case 0x02:		// text, 80x25, 16 cols
			{
				write_regs(g_mode_02);
				setpalette16();
			} break;
		case 0x03:		// text, 80x25, 16 cols
			{
				write_regs(g_mode_03);
				setpalette16();
			} break;
		case 0x04:		// 320x200x4
			{
				write_regs(g_mode_04);
				setpalette4();
			} break;
		case 0x05:		// 320x200x4
			{
				write_regs(g_mode_05);
				/* to make CGA graphics work like other graphics modes... */
				/* 1) turn off screwy CGA addressing */
				outportb(CRT_I, 0x17);
				outportb(CRT_D, inportb(CRT_D) | 1);
				/* 2) turn off doublescan */
				outportb(CRT_I, 9);
				outportb(CRT_D, inportb(CRT_D) & ~0x80);
				/* 3) move the framebuffer from B800:0000 to A000:0000 */
				outportb(GRA_I, 6);
				outportb(GRA_D, inportb(GRA_I) & ~0x0C);
				/* set new address */
				bios_modes[mode].video_ptr = (char *)(0xA0000);
				setpalette4();
			} break;
		case 0x07:		// text, 80x25, 16 cols
			{ 
				write_regs(g_mode_07);
			} break;
		case 0x0D:	// 320x200x16
			{
				write_regs(g_mode_0D);
				setpalette16();
			} break;
		case 0x0E:	// 640x200x16
			{
				write_regs(g_mode_0E);
				setpalette16();
			} break;
		case 0x0F:	// 640x350xMono
			{
				write_regs(g_mode_0F);
			} break;
		case 0x10:	// 640x350x16
			{
				write_regs(g_mode_10);
				setpalette16();
			} break;
		case 0x11:	// 640x480x2		
			{
				write_regs(g_mode_11);
			} break;	
		case 0x12:	// 640x480x16
			{
				write_regs(g_mode_12);
				setpalette16();
			} break;
		case 0x13:	// 320x200x256 aka ModeX
			{
				write_regs(g_mode_13);
				setpalette256();
				// 1) turn off Chain-4 addressing
				// bit 3, memory mode register = 0
				outportb(SEQ_I, 0x04);
				outportb(SEQ_D, inportb(SEQ_D) & ~0x08);
				// 2) turn off doubleword clocking
				// bit 6, underline location register = 0, double word mode off
				outportb(CRT_I, 0x14);
				outportb(CRT_D, inportb(CRT_D) & ~0x40);
				// 3) turn off word clocking in case it's on
				// bit 6, mode control register = 1, byte mode on
				outportb(CRT_I, 0x17);
				outportb(CRT_D, inportb(CRT_D) | 0x40);
			} break;
		default:
			{
				return -0x01;				
			}			
	}
	if (bios_modes[mode].font_height >= 16)
    	memcpy(font_buf, g_8x16_font, 4096);
    else
    	memcpy(font_buf, g_8x8_font, 2048);
		
	if (bios_modes[current_mode].graphic && graphic_card > GR_EGA)
	{
		// restore font data in plane 2
		outportb(SEQ_I, 0x02);
		outportb(SEQ_D, 0x04);
		// restore font data in plane 3, trident
		outportb(SEQ_I, 0x02);
		outportb(SEQ_D, 0x08);
	}
	
	current_mode = mode;
	/* tell the BIOS what we've done, so BIOS text output works OK */		
//	pokew(0x40, 0x4A, bios_modes[mode].max_x);	/* columns on screen */
//	pokew(0x40, 0x4C, bios_modes[mode].max_x * bios_modes[mode].max_y * 2); /* framebuffer size */
//	pokew(0x40, 0x50, 0);		/* cursor pos'n */
//	pokeb(0x40, 0x60, bios_modes[mode].font_height - 1);	/* cursor shape */
//	pokeb(0x40, 0x61, bios_modes[mode].font_height - 2);
//	pokeb(0x40, 0x84, bios_modes[mode].max_x - 1);	/* rows on screen - 1 */
//	pokeb(0x40, 0x85, bios_modes[mode].font_height);		/* char height */
		
	return 0x00;
}

static unsigned int vga_getmode(void)
{
	return current_mode;
}

char find6845(unsigned int addr)
{
	unsigned char tmp, result;

	outportb(addr, 0x0F);
	tmp = inportb(addr + 1);
	
	outportb(addr, 0x0F);
	outportb(addr + 1, 0x66);
	
	delay(100);
	
	result = (inportb(addr + 1) == 0x66);
	outportb(addr + 1, tmp);
	
	return result;
}

char findMono(void)
{
	unsigned char  tmp1, tmp2;
	unsigned int timeout;

	if (find6845(CRT_IM))
	{
		tmp1 = inportb(CRT_IM + 6) & 0x80;
		timeout = 100;
		do {
			tmp2 = inportb(CRT_IM + 6) & 0x80;
			delay(1);
		} while (tmp1 != tmp2 || ! timeout--);
		if (tmp1 != tmp2)
			return GR_HERC;
		return GR_MONO;
	}
	return GR_UNKNOWN;
}

char findCGA(void)
{
	return find6845(CRT_IC) ? GR_CGA : GR_UNKNOWN;
}

static unsigned int vga_test(void)
{
 	unsigned char save, back;
 	
	if (vga_card == GR_UNKNOWN)
	{	// Check if a DAC is present
		save = inportb(PEL_IW);
		graphic_delay();
		outportb(PEL_IW, ~ save);
		graphic_delay();
		back = inportb(PEL_IW);
		graphic_delay();
		outportb(PEL_IW, save);
		save = ~save;
		vga_card = (back == save) ? GR_VGA : GR_EGA;
	}
	if (vga_card == GR_EGA)
	{
		vga_card = findMono();
		if (vga_card == GR_UNKNOWN)
			vga_card = findCGA();
	}
	return 1;	// always true
}

static unsigned int vga_memory(void)
{
	return 32;
}

static char * vga_get_name(void)
{
	switch(vga_card)
	{
		case GR_UNKNOWN:	return "Unknown";
		case GR_MONO:		return "Mono";
		case GR_HERC:		return "HERC";
		case GR_CGA:		return "CGA";
		case GR_EGA:		return "EGA";
		case GR_VGA:		return "VGA";
	}
	return "Unknown";
}

GraphicDriver vga_driver =
{
	vga_test,
	vga_get_name,
	vga_memory,
	vga_setmode,
	vga_getmode,
	NULL,
	NULL,
	NULL,
	NULL,
	vga_putch,
	vga_wherex,
	vga_wherey,
	vga_max_x,
	vga_max_y,
	vga_max_col,
	vga_gotoxy,
	vga_scroll,
	vga_clrscr
};

char Check_VGA(GraphicDriver * driver)
{
	*driver = vga_driver;
	return (vga_test());
}
