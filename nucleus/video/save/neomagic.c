#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define NM_UNKNOWN  0x00
#define NM_2070     0x10
#define NM_2090     0x20
#define NM_2093     0x40
#define NM_2160     0x80

unsigned int neo_chip, neo_mem;

char neo_test(void)
{
	unsigned int sub, x;
	char result;

	result = 0;
	x = rdinx(GRA_I, 9);
	wrinx(GRA_I, 9, 0);
	if (! testinx(GRA_I, 0x15))
	{
		wrinx(GRA_I, 9, 0x26);
		if (testinx(GRA_I, 0x15))
		{
			result = 1;
			sub = rdinx(CRT_I, 0x1A) & 0x1F;
			switch(sub)
			{
				case 0:	neo_chip = NM_2070;
					neo_mem = 896;
					break;
				case 1:	neo_chip = NM_2090;
					neo_mem = 1024;
					break;
				case 2:
				case 3:	neo_chip = NM_2093;
					neo_mem = 1024;
					break;
				case 4:	neo_chip = NM_2160;
					neo_mem = 1984;
					break;
				default: neo_chip = NM_UNKNOWN;
			}
		}
	}
	wrinx(GRA_I, 9, x);
	return result;
}

unsigned int neo_chiptype(void)
{
	return neo_chip;
}

unsigned int neo_memory(void)
{
	return neo_mem;
}

char * neo_get_name(void)
{
	switch(neo_chip)
	{
		case NM_2070 : return "NeoMagic NM2070";
		case NM_2090 : return "NeoMagic NM2090 (128V)";
		case NM_2093 : return "NeoMagic NM2093 (128ZV)";
		case NM_2160 : return "NeoMagic NM2160 (128XD)";
	}
	return "NeoMagic Unknown";
}

void neo_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	if (neo_chip <= NM_2090)
		bank <<= 2;
	wrinx(GRA_I, 0x15, bank << 2);
	wrinx(GRA_I, 0x16, bank << 2);
}

GraphicDriver neomagic_driver =
{
	neo_chiptype,
	neo_get_name,
	neo_memory,
	NULL,
	NULL,
	neo_setbank
};

char Check_NeoMagic(GraphicDriver * driver)
{
	*driver = neomagic_driver;
	return (neo_test());
}
