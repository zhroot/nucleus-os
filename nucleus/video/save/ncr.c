#include <support.h>
#include <stdio.h>
#include <video/graphic.h>
#include <video/graph.h>

#define NCR_UNKNOWN  0x00
#define NCR_77C21    0x10
#define NCR_77C22    0x20
#define NCR_77C22E   0x40
#define NCR_77C22EP  0x80
#define NCR_77C32BLT 0x00

unsigned int ncr_chip, ncr_mem;
char * ncr_addname = "";

char ncr_test(void)
{
	unsigned int sub;

	if (testinx2(SEQ_I, 5, 5))
	{
		wrinx(SEQ_I, 5, 0);	//Disable extended registers
		if (! testinx(SEQ_I, 0x10))
		{
			wrinx(SEQ_I, 5, 1);	//Enable extended registers
			if (testinx(SEQ_I, 0x10))
			{
				sub = rdinx(SEQ_I ,8);
				switch(sub >> 4)
				{
					case 0: ncr_chip = NCR_77C22; break;
					case 1: ncr_chip = NCR_77C21; break;
					case 2: ncr_chip = ((sub & 15) < 8) ? NCR_77C22E : NCR_77C22EP; break;
					case 3: ncr_chip = NCR_77C32BLT; break;
					default: ncr_chip = NCR_UNKNOWN;
				}
//				sprintf(ncr_addname, " Rev. %d", rdinx(SEQ_I, 8) & 15);
//				ncr_mem = check_mem(64, ncr_setbank);
				return 1;
			}
		}
	}
	return 0;
}

unsigned int ncr_chiptype(void)
{
	return ncr_chip;
}

unsigned int ncr_memory(void)
{
	return ncr_mem;
}

char * ncr_get_name(void)
{
	switch(ncr_chip)
	{
		case NCR_77C21    : return "NCR 77c21";
		case NCR_77C22    : return "NCR 77c22";
		case NCR_77C22E   : return "NCR 77c22e";
		case NCR_77C22EP  : return "NCR 77c22e+";
		case NCR_77C32BLT : return "NCR 77c32BLT";
	}
	return "NCR Unknown";
}

void ncr_setbank(unsigned int bank)
{
	if (current_bank == bank)
                return;
        current_bank = bank;
	if (mem_mode <= _pl4)
		bank <<= 2;
	wrinx(SEQ_I, 0x18, bank << 2);
	wrinx(SEQ_I, 0x1C, bank << 2);
}

GraphicDriver ncr_driver =
{
	ncr_chiptype,
	ncr_get_name,
	ncr_memory,
	NULL,
	NULL,
	ncr_setbank
};

char Check_NCR(GraphicDriver * driver)
{
	*driver = ncr_driver;
	return (ncr_test());
}

