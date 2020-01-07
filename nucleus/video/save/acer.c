#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define AC_UNKNOWN 0x00
#define AC_M3141   0x10
#define AC_M3145   0x20

static unsigned int acer_chip, acer_mem;

static char acer_test(void)
{
	unsigned int old, sub;
        char result;

        result = 0;
	old = rdinx(GRA_I, 0xFF);
	clrinx(GRA_I, 0xFF ,7);
	if (! testinx2(GRA_I, 0x10, 0x9F))
        {
		clrinx(GRA_I, 0xFF, 7);
		if (testinx2(GRA_I, 0x10, 0x9F))
		{
                 	result = 1;
			switch(rdinx(GRA_I, 0x12) & 3)
                        {
				case 0: acer_mem = 256; break;
				case 1: acer_mem = 512; break;
				case 2: acer_mem = 1024; break;
				case 3: acer_mem = 2048; break;
			}
			sub = rdinx(GRA_I, 0xFF) >> 6;
			switch (sub)
	                {
				case 2: acer_chip = AC_M3141; break;
				case 3: acer_chip = AC_M3145; break;
	                        default: acer_chip = AC_UNKNOWN;
			}
		}
	}
	wrinx(GRA_I, 0xFF, old);
        return result;
}

static unsigned int acer_chiptype(void)
{
	return acer_chip;
}

static char * acer_get_name(void)
{
	switch(acer_chip)
        {
		case AC_M3141: return "Acer M3141";
		case AC_M3145: return "Acer M3145";
	}
	return "Acer Unknown";
}

static unsigned int acer_memory(void)
{
	return acer_mem;
}

static void acer_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	//Read/Write banks or 32K banks?
	wrinx(GRA_I, 0x10, bank | 0x80);
	wrinx(GRA_I, 0x11, bank | 0x80);
}

GraphicDriver acer_driver =
{
	acer_chiptype,
	acer_get_name,
	acer_memory,
	NULL,
	NULL,
	acer_setbank
};

char Check_Acer(GraphicDriver * driver)
{
	*driver = acer_driver;
	return (acer_test());
}
