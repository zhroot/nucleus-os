#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define SIG_UNKNOWN  0x00
#define SIG_6425     0x10

unsigned int sigma_chip, sigma_mem;

char sigma_test(void)
{
	wrinx(CRT_I, 0x7A, 1);	//Lock
	if (! testinx(CRT_I, 0x45))
	{
		wrinx(CRT_I, 0x7A, 0xAC);	//Unlock
		if (testinx2(CRT_I, 0x45, 0xCF))
		{
			sigma_chip = SIG_6425;
			sigma_mem = (rdinx(CRT_I, 0x57) & 0x70)*64;
			return 1;
		}
	}
	return 0;
}

unsigned int sigma_chiptype(void)
{
	return sigma_chip;
}

unsigned int sigma_memory(void)
{
	return sigma_mem;
}

char * sigma_get_name(void)
{
	switch(sigma_chip)
	{
		case SIG_6425: return "Sigma SD6425";
	}
	return "Sigma Unknown";
}

void sigma_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	wrinx(CRT_I, 0x78, bank);
}

GraphicDriver sigma_driver =
{
	sigma_chiptype,
	sigma_get_name,
	sigma_memory,
	NULL,
	NULL,
	sigma_setbank
};

char Check_Sigma(GraphicDriver * driver)
{
	*driver = sigma_driver;
	return (sigma_test());
}

