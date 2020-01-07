#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define AS_UNKNOWN 0x000
#define AS_3210    0x010
#define AS_6410    0x020
#define AS_6422    0x040
#define AS_6424    0x080
#define AS_643D    0x100

unsigned int alliance_chip, alliance_mem;

static char alliance_test(void)
{
	unsigned int old, sub, x;

	old = rdinx(SEQ_I, 0x10);
	wrinx(SEQ_I, 0x10, 0);
	x = rdinx(SEQ_I, 0x11)*256+rdinx(SEQ_I, 0x12);
	if ((rdinx(SEQ_I ,0x11) == 0x41) && (rdinx(SEQ_I, 0x12) == 0x53))
	{
		if (x == 0x4153)
		{
			if (rdinx(SEQ_I ,0x13) == 0x33)
				alliance_chip = AS_3210;
			else
				alliance_chip = AS_6410;
			outportw(SEQ_I, 0x1210);
			setinx(SEQ_I, 0x1C, 8);
			modinx(SEQ_I, 0x1B, 7, 1);
			_dosmemgetb((0xA000 << 4) + 0xF0, 1, &alliance_mem);
			alliance_mem *= 64;
			clrinx(SEQ_I, 0x1B, 7);
			clrinx(SEQ_I, 0x1C, 8);
		}
		if (x == 0x5072)	//642x+
		{
			sub = rdinx(SEQ_I, 0x16)*256+rdinx(SEQ_I, 0x17);
			switch(sub)
			{
				case 0x3230: alliance_chip = AS_6422; break;
				case 0x3234: alliance_chip = AS_6424; break;
				case 0x3344: alliance_chip = AS_643D; break;
				default: alliance_chip = AS_UNKNOWN;
			}
			alliance_mem = rdinx(SEQ_I, 0x20)*64;
		}
        	return 1;
	}
	return 0;
}

static unsigned int alliance_chiptype(void)
{
	return alliance_chip;
}

static char * alliance_get_name(void)
{
	switch(alliance_chip)
	{
		case AS_3210 : return "Alliance Pro Motion 3210";
		case AS_6410 : return "Alliance Pro Motion 6410";
		case AS_6422 : return "Alliance Pro Motion 6422";
		case AS_6424 : return "Alliance Pro Motion AT24";
		case AS_643D : return "Alliance Pro Motion AT3D";
	}
	return "Alliance Unbekannt";
}

static unsigned int alliance_memory(void)
{
	return alliance_mem;
}

static void alliance_setbank(unsigned int bank)
{
	unsigned int x;

	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (alliance_chip < AS_6422)
	{
		outportw(SEQ_I, 0x1210);
		setinx(SEQ_I, 0x1C, 8);
		modinx(SEQ_I, 0x1B, 7, 1);
		bank <<= 4;
		_dosmemputw(&bank, 1, (0xA000 << 4) + 0xC0);
		clrinx(SEQ_I, 0x1B, 7);
		clrinx(SEQ_I, 0x1C, 8);
	}
	else
	{
		wrinx(SEQ_I, 0x1D, 0x30);	// 0xC0/4
		x = rdinx(SEQ_I, 0x1E)+rdinx(SEQ_I, 0x1F)*256;
		outportw(x, bank << 4);
	}
}

GraphicDriver alliance_driver =
{
	alliance_chiptype,
	alliance_get_name,
	alliance_memory,
	NULL,
	NULL,
	alliance_setbank
};

char Check_Alliance(GraphicDriver * driver)
{
	*driver = alliance_driver;
	return (alliance_test());
}

