#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define HMC_UNKNOWN 0x00
#define HMC_304     0x10
#define HMC_314     0x20

unsigned int hmc_chip, hmc_mem;

char hmc_test(void)
{
	/*  if testinx(SEQ,0xE7) and testinx(SEQ,0xEE) then */
	if (testinx2(SEQ_I, 0xE7, 0x7F) && testinx2(SEQ_I, 0xEE, 0xF1))
	{
		if (rdinx(SEQ_I, 0xE7) & 0x10)
			hmc_mem = 512;
		if (testinx(SEQ_I, 0xE7) && testinx(SEQ_I, 0xEE))
			hmc_chip = HMC_304;
		else
			hmc_chip = HMC_314;
		return 1;
        }
	return 0;
}

unsigned int hmc_chiptype(void)
{
	return hmc_chip;
}

unsigned int hmc_memory(void)
{
	return hmc_mem;
}

char * hmc_get_name(void)
{
	switch(hmc_chip)
	{
		case HMC_304: return  "HMC HM86304";
		case HMC_314: return  "HMC HM86314";
	}
	return "HMC Unknown";
}

void hmc_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (mem_mode == _p8)
		modinx(SEQ_I, 0xEE, 0x70, bank << 4);
}

GraphicDriver hmc_driver =
{
	hmc_chiptype,
	hmc_get_name,
	hmc_memory,
	NULL,
	NULL,
	hmc_setbank
};

char Check_HMC(GraphicDriver * driver)
{
	*driver = hmc_driver;
	return (hmc_test());
}

