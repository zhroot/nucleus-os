#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define ARK_UNKNOWN 0x000
#define ARK1000VL   0x010
#define ARK1000PV   0x020
#define ARK2000PV   0x040
#define ARK2000MT   0x080
#define ARK2000MI   0x100

unsigned int ark_chip, ark_mem, ark_ioaddr;

static char ark_test(void)
{
	unsigned int old, sub;
	char result;

	result = 0;
	old = rdinx(SEQ_I, 0x1D);
	wrinx(SEQ_I, 0x1D, 0);	// Lock the ext registers
	if (! (testinx(SEQ_I, 0x11) && testinx(SEQ_I, 0x12)))
	{
		wrinx(SEQ_I, 0x1D, old);
		if (testinx(SEQ_I, 0x11) && testinx(SEQ_I, 0x12))
		{
			result = 1;
			sub = rdinx(CRT_I, 0x50);
			switch(sub & 0xF8)
			{
				case 0x88: ark_chip = ARK1000VL; break;
				case 0x90: ark_chip = ARK1000PV; break;
				case 0x98: ark_chip = ARK2000PV; break;
				case 0xA0: ark_chip = ARK2000MT; break;
				case 0xA8: ark_chip = ARK2000MI; break;
				default: ark_chip = ARK_UNKNOWN;
			}
			if (ark_chip >= ARK2000PV)
			{
				switch(rdinx(SEQ_I, 0x10) >> 6)
				{
					case 0: ark_mem = 1024; break;
					case 1: ark_mem = 2048; break;
					case 2: ark_mem = 4096; break;
					case 3: ark_mem = 8192; break;
				}
			}
                	else
			if (rdinx(SEQ_I,0x10) & 0x40)
				ark_mem = 2048;
			else
				ark_mem = 1024;
		}
	}
	wrinx(SEQ_I, 0x1D, old);
	return result;
}

static unsigned int ark_chiptype(void)
{
	return ark_chip;
}

static unsigned int ark_memory(void)
{
	return ark_mem;
}

static char * ark_get_name(void)
{
	switch(ark_chip)
	{
		case ARK1000VL: return "1000VL";
		case ARK1000PV: return "1000PV";
		case ARK2000PV: return "2000PV";
		case ARK2000MT: return "2000MT";
		case ARK2000MI: return "2000MI";
	}
	return "Ark Unknown";
}

static void ark_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	wrinx(SEQ_I, 0x15, bank);
	wrinx(SEQ_I, 0x16, bank);
}

GraphicDriver ark_driver =
{
	ark_chiptype,
	ark_get_name,
	ark_memory,
	NULL,
	NULL,
	ark_setbank
};

char Check_Ark(GraphicDriver * driver)
{
	*driver = ark_driver;
	return (ark_test());
}
