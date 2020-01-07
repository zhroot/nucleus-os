#include <support.h>
#include <stdio.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <video/fonts.h>
#include <drivers/pci.h>
#include <drivers/timer.h>

unsigned int CRT_I = CRT_IM;
unsigned int CRT_D = CRT_DM;
unsigned int IS1_R = IS1_RM;
unsigned int graphic_card = GR_UNKNOWN;

static GraphicDriver graphic_driver, real_driver;

char GraphicMode(void)
{
	outportb(GRA_I, 0x06);
	return (inportb(GRA_D) & 0x01);
}

void GetCRTC(void)
{
	if (inportb(MIS_R) & 1)
	{
		CRT_I = CRT_IC;
		IS1_R = IS1_RC;
	}
	else
	{
		CRT_I = CRT_IM;
		IS1_R = IS1_RM;
	}
	CRT_D = CRT_I + 1;
}

void SetCRTC(int color)
{
	if (color)
	{
		CRT_I = CRT_IC;
		IS1_R = IS1_RC;
	}
	else
	{
		CRT_I = CRT_IM;
		IS1_R = IS1_RM;
	}
	CRT_D = CRT_I + 1;
}

char set_mode(unsigned int mode)
{
	char res = -1;
	
	if (graphic_driver.SetMode != NULL)
	{
		// screen off
//		if (graphic_card > GR_EGA)
//		{
//		    outportb(SEQ_I, 0x01);
//	    	outportb(SEQ_D, inportb(SEQ_D) | 0x20);
//		}
		// Disable video output
		(void)inportb(IS1_RC);
		delay(10);
		outportb(ATT_I, 0x00);
		
		// vertical retrace
//		while (((inportb(IS1_RC)) & 0x08) == 0x08 );
//		while (((inportb(IS1_RC)) & 0x08) == 0 ); 
	   		
//		res = graphic_driver.SetMode(mode);
		
		// screen on
//		if (graphic_card > GR_EGA)
//		{
//	    	outportb(SEQ_I, 0x01);
//	    	outportb(SEQ_D, inportb(SEQ_D) & 0xDF); 
//  	}
  		// enable video output 	
		(void)inportb(IS1_RC);
		delay(10);
		outportb(ATT_I, 0x20);
		
	}
	return res;
}

void putch(unsigned int x, unsigned int y, unsigned char b)
{
	if (graphic_driver.WriteChar != NULL)
		graphic_driver.WriteChar(x, y, b);
}

unsigned int wherex(void)
{
	if (graphic_driver.GetX != NULL)
		return graphic_driver.GetX();
	return 0;
}

unsigned int wherey(void)
{
	if (graphic_driver.GetY != NULL)
		return graphic_driver.GetY();
	return 0;
}

void gotoxy(unsigned int x, unsigned int y)
{
	if (graphic_driver.GotoXY != NULL)
		graphic_driver.GotoXY(x, y);
}

void clrscr(void)
{
	if (graphic_driver.ClrScr != NULL)
		graphic_driver.ClrScr();
}

unsigned int max_x(void)
{
	if (graphic_driver.GetMaxX != NULL)
		return graphic_driver.GetMaxX();
	return 0;
}

unsigned int max_y(void)
{
	if (graphic_driver.GetMaxY != NULL)
		return graphic_driver.GetMaxY();
	return 0;
}

void scroll(unsigned int lines)
{
	if (graphic_driver.Scroll != NULL)
		graphic_driver.Scroll(lines);
}

int Init_Graphic(void)
{
	if (GraphicMode())
		printf("graphic: graphic mode active\n\r");
//	if (Check_3dfx(&real_driver))		return GR_3Dfx;
//	if (Check_3dlabs(&real_driver))		return GR_3DLabs;
//	if (Check_Acer(&real_driver))		return GR_Acer;
//	if (Check_Ahead(&real_driver))		return GR_Ahead;
//	if (Check_ALG(&real_driver))		return GR_ALG;
//	if (Check_Alliance(&real_driver))	return GR_Alliance;
//	if (Check_Ark(&real_driver))		return GR_Ark;
//	if (Check_Ati(&real_driver))		return GR_Ati;
//	if (Check_Cirrus(&real_driver))		return GR_Cirrus;
//	if (Check_Compaq(&real_driver))		return GR_Compaq;
//	if (Check_Cyrix(&real_driver))		return GR_Cyrix;
//	if (Check_Genoa(&real_driver))		return GR_Genoa;
//	if (Check_HiQ(&real_driver))		return GR_HiQ;
//	if (Check_HMC(&real_driver))		return GR_HMC;
//	if (Check_Imagine(&real_driver))	return GR_Imagine;
//	if (Check_Mach32(&real_driver))		return GR_Mach32;
//	if (Check_Mach64(&real_driver))		return GR_Mach64;
//	if (Check_MXIC(&real_driver))		return GR_MXIC;
//	if (Check_NCR(&real_driver))		return GR_NCR;
//	if (Check_NeoMagic(&real_driver))	return GR_NeoMagic;
//	if (Check_Nvidia(&real_driver))		return GR_Nvidia;
//	if (Check_OAK(&real_driver))		return GR_OAK;
//	if (Check_OPTi(&real_driver))		return GR_OPTi;
//	if (Check_Paradise(&real_driver))	return GR_Paradise;
//	if (Check_P2000(&real_driver))		return GR_P2000;
//	if (Check_Realtek(&real_driver))	return GR_Realtek;
//	if (Check_Rendition(&real_driver))	return GR_Rendition;
//	if (Check_Sierra(&real_driver))		return GR_Sierra;
//	if (Check_Sigma(&real_driver))		return GR_Sigma;
//	if (Check_SiS(&real_driver))		return GR_SiS;
//	if (Check_SMOS(&real_driver))		return GR_SMOS;
//	if (Check_S3(&real_driver))			return GR_S3;
//	if (Check_Trident(&real_driver))	return GR_Trident;
//	if (Check_TSENG(&real_driver))		return GR_TSENG;
//	if (Check_UMC(&real_driver))		return GR_UMC;
//	if (Check_Video7(&real_driver))		return GR_Video7;
	if (Check_VGA(&real_driver))		return GR_VGA;
	return 0x00;
}

void get_char_graph(unsigned char ch, unsigned int * chardata)
{
	int i;

	outportw(0x3C4, 0x0402);
	outportw(0x3C4, 0x0704);
	outportw(0x3CE, 0x0204);
	outportw(0x3CE, 0x0005);
	outportw(0x3CE, 0x0006);
	for (i=0; i<16; i++)
		_dosmemgetw(0xB8000 + (ch << 5), 1, &chardata[i]);
	outportw(0x3C4, 0x0302);
	outportw(0x3C4, 0x0304);
	outportw(0x3CE, 0x0004);
	outportw(0x3CE, 0x1005);
	outportw(0x3CE, 0x0E06);
}

void set_char_graph(unsigned char ch, unsigned int * chardata)
{
	int i;

	outportw(0x3C4, 0x0402);
	outportw(0x3C4, 0x0704);
	outportw(0x3CE, 0x0204);
	outportw(0x3CE, 0x0005);
	outportw(0x3CE, 0x0006);
	for (i=0; i<16; i++)
		_dosmemputw(&chardata[i], 1, 0xB8000 + (ch << 5));
	outportw(0x3C4, 0x0302);
	outportw(0x3C4, 0x0304);
	outportw(0x3CE, 0x0004);
	outportw(0x3CE, 0x1005);
	outportw(0x3CE, 0x0E06);
}

void graphic_info(void)
{
	if (real_driver.WriteChar == NULL || real_driver.GotoXY == NULL || 
		real_driver.GetX == NULL || real_driver.GetY == NULL || 
		real_driver.Scroll == NULL || real_driver.ClrScr == NULL)
	{
		if (real_driver.DetectName != NULL && real_driver.DetectMemory != NULL)
		{
			printf("graphic: Found card: %s (%dKB)\n", real_driver.DetectName(), real_driver.DetectMemory());
			printf("graphic: Found supported card, but not all functions supported by driver now.\n");
		}
	}
	printf("graphic: %s (%dKB)\n", graphic_driver.DetectName(), graphic_driver.DetectMemory());
}

char graphic_init(void)
{
	graphic_card = GR_VGA;
	graphic_driver = vga_driver;	
	SetCRTC(1);
	
	if (CheckPCI())
		printf("graphic: PCI configuration type %d (%d devices)", cfg_mech, cfg_max);
	graphic_card = Init_Graphic();
	if (real_driver.WriteChar == NULL || real_driver.GotoXY == NULL || 
		real_driver.GetX == NULL || real_driver.GetY == NULL || 
		real_driver.Scroll == NULL || real_driver.ClrScr == NULL)
	{
		printf("graphic: Found supported card, but not all functions supported by driver now.\n");
		printf("         Switching to text mode driver.\n");
	}
	else
	{
		graphic_driver = real_driver;
		graphic_driver.SetMode(0x3);	
		
//		int x, y;
//		for (x=0; x<10; x++)
//			for (y=0; y<10; y++)
//				graphic_driver.WritePixel(x, y, CL_WHITE);
	}
	return 0;
}
