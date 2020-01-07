#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define OPTi_UNKNOWN  0x000
#define OPTi_168      0x010
#define OPTi_178      0x020
#define OPTi_264      0x040
#define OPTi_265      0x080
#define OPTi_268      0x100

unsigned int opti_chip, opti_mem;

char opti_test(void)
{
	unsigned int x, y;
	char result;

	result = 0;
	y = rdinx(SEQ_I, 0x10);
	wrinx(SEQ_I, 0x10, 0);
	if (! testinx2(GRA_I, 0x20,0xF))
	{
		wrinx(SEQ_I, 0x10, 0xAA);
		if (testinx2(GRA_I, 0x20, 0xF))
		{
			result = 1;
			x = rdinx(CRT_I, 0x28)*16+(rdinx(CRT_I, 0x29) >> 4);
			switch(x)
			{
				case 0x328: opti_chip = OPTi_168; break;
				case 0x178:
					opti_chip = (rdinx(CRT_I, 0x29) == 0x80) ? OPTi_178 : OPTi_264; break;
				case 0x264: opti_chip = OPTi_264; break;
				case 0x265: opti_chip = OPTi_265; break;
				case 0x268: opti_chip = OPTi_268; break;
			}
			switch(rdinx(CRT_I, 0x19) & 3)
			{
				case 0: opti_mem = 512; break;
				case 1: opti_mem = 1024; break;
				case 2: opti_mem = 2048; break;
				case 3: opti_mem = 4096; break;
			}
		}
	}
	wrinx(SEQ_I, 0x10, y);
	return result;
}

unsigned int opti_chiptype(void)
{
	return opti_chip;
}

unsigned int opti_memory(void)
{
	return opti_mem;
}

char * opti_get_name(void)
{
	switch(opti_chip)
	{
		case OPTi_168 : return "OPTi 92c168";
		case OPTi_178 : return "OPTi 92c178";
		case OPTi_264 : return "OPTi 82c264";
		case OPTi_265 : return "OPTi 82c265";
		case OPTi_268 : return "OPTi 82c268";
	}
	return "OPTi Unknown";
}

void opti_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	wrinx(GRA_I, 0x20, bank);
	wrinx(GRA_I, 0x21, bank);
}


GraphicDriver opti_driver =
{
	opti_chiptype,
	opti_get_name,
	opti_memory,
	NULL,
	NULL,
	opti_setbank
};

char Check_OPTi(GraphicDriver * driver)
{
	*driver = opti_driver;
	return (opti_test());
}
