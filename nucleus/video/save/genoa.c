#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define GVGA_UNKNOWN  0x00
#define GVGA_5100     0x10	// Tseng ET3000
#define GVGA_5300     0x20	// Tseng ET3000
#define GVGA_6100     0x30	// Genoa GVGA
#define GVGA_6200     0x40	// Genoa GVGA
#define GVGA_6400     0x50	// Genoa GVGA

unsigned int genoa_chip, genoa_mem;

char genoa_test(void)
{
	unsigned char add0, add1, add2, add3;
	unsigned char biosptr[3];
	char result;

	result = 0;
  	_dosmemgetb(0xC0000, 4, &biosptr);
	_dosmemgetb((0xC000 << 4) + biosptr[0], 1, &add0);
	_dosmemgetb((0xC000 << 4) + biosptr[1], 1, &add1);
	_dosmemgetb((0xC000 << 4) + biosptr[2], 1, &add2);
	_dosmemgetb((0xC000 << 4) + biosptr[3], 1, &add3);
	if (add0 == 0x77 && add2 == 0x66 && add3 == 0x99)
	{
		result = 1;
		switch(add1)
		{
			case 0x33: genoa_chip = GVGA_5100; genoa_mem = 256; break;
			case 0x55: genoa_chip = GVGA_5300; genoa_mem = 512; break;
			case 0x22: genoa_chip = GVGA_6100; genoa_mem = 256; break;
			case 0x00: genoa_chip = GVGA_6200; genoa_mem = 256; break;
			case 0x11: genoa_chip = GVGA_6400; genoa_mem = 512; break;
			default: genoa_chip = GVGA_UNKNOWN; genoa_mem = 256;
		}
        }
	return result;
}

unsigned int genoa_chiptype(void)
{
	return genoa_chip;
}

char * genoa_get_name(void)
{
	switch(genoa_chip)
	{
		case GVGA_5100: return "Genoa 5100/5200";
		case GVGA_5300: return "Genoa 5300/5400";
		case GVGA_6100: return "Genoa 6100";
		case GVGA_6200: return "Genoa 6200/6300";
		case GVGA_6400: return "Genoa 6400/6600";
	}
	return "Genoa Unknown";
}

unsigned int genoa_memory(void)
{
	return genoa_mem;
}

void genoa_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	wrinx(SEQ_I, 6, bank*9+0x40);
}

GraphicDriver genoa_driver =
{
	genoa_chiptype,
	genoa_get_name,
	genoa_memory,
	NULL,
	NULL,
	genoa_setbank
};

char Check_Genoa(GraphicDriver * driver)
{
	*driver = genoa_driver;
	return (genoa_test());
}
