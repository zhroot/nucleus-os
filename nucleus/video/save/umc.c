#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define MC_UNKNOWN  0x00
#define UMC_418     0x10
#define UMC_408     0x20

unsigned int umc_chip, umc_mem;

char umc_test(void)
{
	unsigned int old;
	char result;

	result = 0;
	old = inportb(0x3BF);
	outportb(0x3BF, 3);
	if (! testinx(SEQ_I, 6))
	{
		outportb(0x3BF, 0xAC);
		if (testinx(SEQ_I, 6))
		{
			result = 1;
			switch(rdinx(SEQ_I, 7) >> 6)
			{
			 	case 1: umc_mem = 512; break;
				case 2:
 				case 3: umc_mem = 1024; break;
			}
			if (testinx2(CRT_I, 0x35, 0xF))
			{
				umc_chip = UMC_418;
				if ((rdinx(GRA_I, 0xB) & 0x7F) == 0x2A)
					umc_mem = 1024;
			}
                	else
				umc_chip = UMC_408;
		}
	}
	outportb(0x3BF, old);
	return result;
}

unsigned int umc_chiptype(void)
{
	return umc_chip;
}

unsigned int umc_memory(void)
{
	return umc_mem;
}

char * umc_get_name(void)
{
	switch(umc_chip)
	{
		case UMC_408: return "UMC UM85C408";
		case UMC_418: return "UMC UM85C418";
	}
	return "UMC Unknown";
}

void umc_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	wrinx(SEQ_I, 6, bank*17);
}

GraphicDriver umc_driver =
{
	umc_chiptype,
	umc_get_name,
	umc_memory,
	NULL,
	NULL,
	umc_setbank
};

char Check_UMC(GraphicDriver * driver)
{
	char res;
	
	res = umc_test();
	if (res)
		*driver = umc_driver;
	return res;
}

