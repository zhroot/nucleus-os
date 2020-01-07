#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define SM_UNKNOWN  0x000
#define SM_8104     0x010
#define SM_8106     0x020
#define SM_8107     0x040
#define SM_8108     0x080
#define SM_8110     0x100

unsigned int smos_chip, smos_mem;

char smos_test(void)
{
 	unsigned int x, y;
	char result;

	result = 0;
  	x = rdinx(0x3DE, 0xE);
	wrinx(0x3DE, 0xE, 0x55);	//disable
	if (! testinx(0x3DE, 0xD))
	{
		wrinx(0x3DE, 0xE, 0x1A);	//disable
		y = inportb(0x3DF);
		if (testinx(0x3DE, 0xD))
		{
			result = 1;
			switch(rdinx(0x3DE, 0xF) >> 3)
			{
				case 2: smos_chip = SM_8104; break;
				case 4: smos_chip = SM_8108; break;
				case 20: smos_chip = SM_8110; break;
			}
			smos_mem = 256;
		}
	}
	wrinx(0x3DE, 0xE, x);
	return result;
}

unsigned int smos_chiptype(void)
{
	return smos_chip;
}

unsigned int smos_memory(void)
{
	return smos_mem;
}

char * smos_get_name(void)
{
	switch(smos_chip)
	{
		case SM_8104 : return "S-MOS SPC8104";
		case SM_8106 : return "S-MOS SPC8107";
		case SM_8107 : return "S-MOS SPC8108";
		case SM_8108 : return "S-MOS SPC81092";
		case SM_8110 : return "S-MOS SPC8110";
	}
	return "S-MOS Unknown";
}

void smos_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportb(0x3CD, bank*17);
}

GraphicDriver smos_driver =
{
	smos_chiptype,
	smos_get_name,
	smos_memory,
	NULL,
	NULL,
	smos_setbank
};

char Check_SMOS(GraphicDriver * driver)
{
	*driver = smos_driver;
	return (smos_test());
}

