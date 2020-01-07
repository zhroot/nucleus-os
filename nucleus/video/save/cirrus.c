#include <support.h>
#include <string.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <video/cirrus.h>
#include <video/cirrus54.h>
#include <video/cirrus64.h>

unsigned int cirrus_chip, cirrus_mem;
char cirrus_other;
// Forward reference
extern GraphicDriver _cirrus_driver;

char cirrus_test(void)
{
	unsigned int old, old6, sub;
	char result;

	result = 0;
	if (Check_Cirrus54(&_cirrus_driver))
	{
		cirrus_other = 1;
                return 1;
        }
	if (Check_Cirrus64(&_cirrus_driver))
	{
		cirrus_other = 1;
                return 1;
        }
	old6 = rdinx(SEQ_I, 6);
	old = rdinx(CRT_I, 0xC);
	outportb(CRT_I+1, 0);
	sub = rdinx(CRT_I, 0x1F);
	wrinx(SEQ_I, 6, LOBYTE(sub >> 4) | LOBYTE(sub << 4));
	//The SubVers value is rotated by 4
	if (! inportb(SEQ_I+1))
        {
		cirrus_mem = 256;
		outportb(0x3c5, sub);
		if (inportb(0x3c5) == 1)
		{
			result = 1;
			switch(sub)
                        {
				case 0xEC: cirrus_chip = CL_GD5x0; break;
				case 0xCA: cirrus_chip = CL_GD6x0; break;
				case 0xEA: cirrus_chip = CL_V7_OEM; break;
				default: cirrus_chip = CL_OLD_UNKNOWN;
			}
		}
	}
	wrinx(CRT_I, 0xC ,old);
	wrinx(SEQ_I, 6, old6);
	return result;
}

unsigned int cirrus_chiptype(void)
{
	unsigned int result;

        result = _cirrus_driver.DetectChip();
	if (result == CL_UNKNOWN)
		result = cirrus_chip;
	return result;
}

unsigned int cirrus_memory(void)
{
	unsigned int result;

        result = _cirrus_driver.DetectMemory();
	if (result == 0)
		result = cirrus_mem;
	return result;
}

char * cirrus_get_name(void)
{
	char * result;

        result = _cirrus_driver.DetectName();
	if (strstr(result, "Unknown"))
	{
		switch(cirrus_chip)
                {
			case CL_GD5x0: return "Cirrus CL-GD 54xx";
			case CL_GD6x0: return "Cirrus CL-GD 64xx";
			case CL_V7_OEM: return "Cirrus CL-V7 OEM";
			case CL_OLD_UNKNOWN: return "Cirrus Unknwon (too old ?)";
		}
		return "Cirrus Unknown";
        }
	return result;
}

GraphicDriver _cirrus_driver =
{
	cirrus_chiptype,
	cirrus_get_name,
	cirrus_memory
};

char Check_Cirrus(GraphicDriver * driver)
{
	*driver = _cirrus_driver;
	cirrus_other = 0;
	if (cirrus_test())
	{
		if (cirrus_other)
			*driver = _cirrus_driver;
		return 1;
	}
	return 0;
}
