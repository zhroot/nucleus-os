#include <video/graphic.h>
#include <video/graph.h>
#include <drivers/pci.h>
#include <support.h>

#define SIS_UNKNOWN 0x00
#define SIS_201	 	0x01
#define SIS_530	 	0x02
#define SIS_6202	0x03
#define SIS_6205	0x04
#define SIS_6215	0x05
#define SIS_6225	0x06
#define SIS_5596	0x07
#define SIS_5598	0x08
#define SIS_6326	0x09

unsigned int sis_chip, sis_mem;

char sis_test(void)
{
	unsigned int old, sub;
	unsigned char counter;

	old = rdinx(SEQ_I, 5);
	wrinx(SEQ_I, 5, 0);
	if (rdinx(SEQ_I, 5) == 0x21)
	{
		wrinx(SEQ_I, 5, 0x86);
		if ((rdinx(SEQ_I, 5) == 0xA1))
		{
			if (CheckPCI() && GetSpecialVendor(0x1039))
			{
				for (counter=0; counter<cfg_max; counter++)
				{
					if (pci_list[counter].vendorID != 0x1039)
						continue;
					sub = pci_list[counter].deviceID;
					switch (sub)
					{
						case 1: sis_chip = SIS_201; break;
						case 2: sis_chip = SIS_6202; break;
						case 0x200: sis_chip = SIS_5598; break;
						case 0x205: switch (pci_list[counter].revisionID)
									{
										case 13: sis_chip = SIS_6205; break;
										case 14: sis_chip = SIS_5596; break;
										default: sis_chip = SIS_6215;
									}
						case 0x0530: sis_chip = SIS_530; break;
						case 0x6225: sis_chip = SIS_6225; break;
						case 0x6326: sis_chip = SIS_6326; break;
					}
					sis_mem = 0;
					switch (rdinx(SEQ_I, 0xC) & 6)
					{
						case 0: sis_mem = 1024; break;
						case 2: sis_mem = 2048; break;
						case 4: sis_mem = 4096; break;
						case 6: sis_mem = 8192; break;
					}
					if (! sis_mem)
					switch (rdinx(SEQ_I, 0xF) & 3)
					{
						case 0: sis_mem = 1024; break;
						case 1: sis_mem = 2048; break;
						case 2: sis_mem = 4096; break;
						// 5596/98 shared memory
						case 3: sis_mem = (rdinx(SEQ_I, 0x25) >> 4)*512+512; break;
					}
					if (sis_chip) break;
				}
			}
		}
	}
	wrinx(SEQ_I, 5, old);
	return (sis_chip != 0);
}

char * sis_get_name(void)
{
	switch (sis_chip)
	{
		case SIS_201  : return "SiS SG86c201";
		case SIS_530  : return "SiS 530";
		case SIS_6202 : return "SiS 6202";
		case SIS_6205 : return "SiS 6205";
		case SIS_6215 : return "SiS 6215";
		case SIS_6225 : return "SiS 6225";
		case SIS_6326 : return "SiS 6326";
		case SIS_5596 : return "SiS 5596";
		case SIS_5598 : return "SiS 5598";
	}
	return "SiS Unknown";
}

void sis_setbank(unsigned int bank)
{
	if (current_bank == bank)
		return;
	current_bank = bank;
	outportb(0x3CD, bank);
	outportb(0x3CB, bank);
}

unsigned int sis_chiptype(void)
{
	return sis_chip;
}

unsigned int sis_memory(void)
{
	return sis_mem;
}

GraphicDriver sis_driver =
{
	sis_chiptype,
	sis_get_name,
	sis_memory,
	NULL,
	NULL,
	sis_setbank
};

char Check_SiS(GraphicDriver * driver)
{
	*driver = sis_driver;
	return (sis_test());
}
