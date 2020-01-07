#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define Chip_Intel  0x01
#define Chip_HiQ    0x02

#define In_UNKNOWN  0x00
#define HiQ_UNKNOWN 0x01

#define In_740      0x10

#define HiQ_65550   0x20
#define HiQ_65554   0x30

unsigned int hiq_chip, hiq_chip_v, hiq_mem;

char hiq_test(void)
{
 	unsigned int sub, x;

	x = rdinx2(0x3D6, 0);
	sub = rdinx2(0x3D6, 2);
	if (x == 0x8086)	// Intel
	{
		hiq_chip_v = Chip_Intel;
		switch(sub)
		{
			case 0x7800: hiq_chip = In_740; break;
			default: hiq_chip = In_UNKNOWN;
		}
		hiq_mem = ((rdinx(0x3D6, 0xE0) & 15)+1)*512;
		return 1;
	}
	else
	if (x == 0x102C)	// Chips
	{
		hiq_chip_v = Chip_HiQ;
		switch(sub)
		{
			case 0xE0: hiq_chip = HiQ_65550; break;
			case 0xE4: hiq_chip = HiQ_65554; break;
			default: hiq_chip = HiQ_UNKNOWN;
		}
		hiq_mem = ((rdinx(0x3D6, 0xE0) & 15)+1)*512;
		return 1;
	}
	return 0;
}

unsigned int hiq_chiptype(void)
{
	return hiq_chip == 1 ? 0 : hiq_chip;
}

unsigned int hiq_memory(void)
{
	return hiq_mem;
}

char * hiq_get_name(void)
{
	switch(hiq_chip)
	{
		case In_UNKNOWN : return "Intel Unknown";
		case In_740     : return "Intel 740";
		case HiQ_UNKNOWN: return "HiQ Unknown";
		case HiQ_65550  : return "HiQ 65550";
		case HiQ_65554  : return "HiQ 65554";
	}
	return "Intel/HiQ Unknown";
}

void hiq_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (hiq_chip_v == Chip_HiQ)
		wrinx(0x3D6, 0xE, bank);
	else	// Intel
		wrinx(0x3D6, 0xE, bank);
}

GraphicDriver hiq_driver =
{
	hiq_chiptype,
	hiq_get_name,
	hiq_memory,
	NULL,
	NULL,
	hiq_setbank
};

char Check_HiQ(GraphicDriver * driver)
{
	*driver = hiq_driver;
 	return (hiq_test());
}

