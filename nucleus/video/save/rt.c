#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define RT_UNKNOWN  0x00
#define RT_3103     0x10
#define RT_3105     0x20
#define RT_3106     0x40

unsigned int realtek_chip, realtek_mem;

char realtek_test(void)
{
	unsigned int sub;

	if (testinx2(CRT_I, 0x1F, 0x3F) &&
		tstrg(0x3D6, 0xF) && tstrg(0x3D7, 0xF))
	{
		sub = rdinx(CRT_I, 0x1A) >> 6;
		switch(sub)
		{
			case 0: realtek_chip = RT_3103; break;
			case 1: realtek_chip = RT_3105; break;
			case 2: realtek_chip = RT_3106; break;
			default: realtek_chip = RT_UNKNOWN;
		}
		switch(rdinx(CRT_I, 0x1E) & 15)
		{
			case 0: realtek_mem = 256; break;
			case 1: realtek_mem = 512; break;
			case 2: realtek_mem = (sub == 0) ? 768 : 1024; break;
			case 3: realtek_mem = (sub == 0) ? 1024 : 2048; break;
		}
	}
	return realtek_chip;
}

unsigned int realtek_chiptype(void)
{
	return realtek_chip;
}

unsigned int realtek_memory(void)
{
	return realtek_mem;
}

char * realtek_get_name(void)
{
	switch(realtek_chip)
	{
		case RT_3103 : return "Realtek RT3103";
		case RT_3105 : return "Realtek 3105";
		case RT_3106 : return "Realtek 3106";
	}
	return "Realtek Unknown";
}

void realtek_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportb(0x3d6, bank);
	outportb(0x3d7, bank);
}

GraphicDriver realtek_driver =
{
	realtek_chiptype,
	realtek_get_name,
	realtek_memory,
	NULL,
	NULL,
	realtek_setbank
};

char Check_Realtek(GraphicDriver * driver)
{
	*driver = realtek_driver;
	return (realtek_test());
}
