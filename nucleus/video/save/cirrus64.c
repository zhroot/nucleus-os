#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <video/cirrus.h>
#include <video/cirrus64.h>

unsigned int cirrus64_chip, cirrus64_mem;

static char cirrus64_test(void)
{
	unsigned int old, sub;
        char result;

        result = 0;
	old = rdinx(GRA_I, 0xA);
	wrinx(GRA_I, 0xA, 0xCE);  //Lock
	if (! rdinx(GRA_I, 0xA))
	{
		wrinx(GRA_I, 0xA, 0xEC);  //unlock
		if (rdinx(GRA_I, 0xA) == 1)
                {
                 	result = 1;
			sub = rdinx(GRA_I, 0xAA);
			switch(sub >> 4)
                        {
				case 4: cirrus64_chip = CL_GD6440; break;
				case 5: cirrus64_chip = CL_GD6412; break;
				case 6: cirrus64_chip = CL_GD5410; break;
				case 7: cirrus64_chip = (testinx2(GRA_I, 0x87, 0x90)) ? CL_GD6420B : CL_GD6420A; break;
				case 8: cirrus64_chip = CL_GD6410; break;
				default: cirrus64_chip = CL_UNKNOWN;
			}
			switch(rdinx(GRA_I, 0xBB) >> 6)
                        {
				case 0: cirrus64_mem = 256; break;
				case 1: cirrus64_mem = 512; break;
				case 2: cirrus64_mem = 768; break;
				case 3: cirrus64_mem = 1024; break;
			}
		}
	}
	wrinx(GRA_I, 0xA, old);
        return result;
}

static unsigned int cirrus64_chiptype(void)
{
	return cirrus64_chip;
}

static unsigned int cirrus64_memory(void)
{
	return cirrus64_mem;
}

static char * cirrus64_get_name(void)
{
	switch(cirrus64_chip)
        {
		case CL_GD5410 : return "Cirrus CL-GD5410";
		case CL_GD6410 : return "Cirrus CL-GD6410";
		case CL_GD6412 : return "Cirrus CL-GD6412";
		case CL_GD6420A: return "Cirrus CL-GD6420A";
		case CL_GD6420B: return "Cirrus CL-GD6420B";
		case CL_GD6440 : return "Cirrus CL-GD6440";
	}
	return "Cirrus Unknown";
}

static void cirrus64_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	bank <<= 4;
	wrinx(GRA_I, 0xE, bank);
	wrinx(GRA_I, 0xF, bank);
}

GraphicDriver cirrus64_driver =
{
	cirrus64_chiptype,
	cirrus64_get_name,
	cirrus64_memory,
	NULL,
	NULL,
	cirrus64_setbank
};

char Check_Cirrus64(GraphicDriver *driver)
{
	if (cirrus64_test())
	{
		*driver = cirrus64_driver;
		return 1;
        }
	return 0;
}
