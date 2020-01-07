#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define SC_UNKNOWN  0x00
#define SC_15064    0x10		// Sierra Falcon/64

unsigned int sierra_chip, sierra_mem;

char sierra_test(void)
{
	unsigned int old, i;
	char result;

	result = 0;
	old = rdinx(SEQ_I, 0x11);
	setinx(SEQ_I, 0x11, 0x20);
	if (! testinx(SEQ_I, 0x15))
	{
		i = rdinx(SEQ_I, 0x11);
		outportb(SEQ_I+1, i);
		outportb(SEQ_I+1, i);
		outportb(SEQ_I+1, i & 0xDF);
		if (testinx(SEQ_I, 0x15))
		{
			result = 1;
			// setinx(SEQ_I, 0x11,0x20);
			switch(rdinx(SEQ_I, 7) >> 5)
			{
				case  4: sierra_chip = SC_15064; break;
				default: sierra_chip = SC_UNKNOWN;
			}
			switch(rdinx(SEQ_I, 0x12) & 0xD0)
			{
				case 0x10: sierra_mem = 2560; break;
				case 0x40: sierra_mem = 512; break;
				case 0x50: sierra_mem = 3072; break;
				case 0x80: sierra_mem = 1024; break;
				case 0x90: sierra_mem = 4096; break;
				case 0xC0: sierra_mem = 2048; break;
			}
		}
	}
	wrinx(SEQ_I, 0x11, old);
	return result;
}

unsigned int sierra_chiptype(void)
{
	return sierra_chip;
}

unsigned int sierra_memory(void)
{
	return sierra_mem;
}

char * sierra_get_name(void)
{
	switch(sierra_chip)
	{
		case SC_15064: return "Sierra Falcon/64 (SC15064)";
	}
	return "Sierra Unknown";
}

void sierra_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportb(0x3CD, bank << 1);
	wrinx(SEQ_I, 0x15, bank << 1);
}

GraphicDriver sierra_driver =
{
	sierra_chiptype,
	sierra_get_name,
	sierra_memory,
	NULL,
	NULL,
	sierra_setbank
};

char Check_Sierra(GraphicDriver * driver)
{
	*driver = sierra_driver;
	return (sierra_test());
}

