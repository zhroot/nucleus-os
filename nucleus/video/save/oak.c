#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define OAK_UNKNOWN  0x00
#define OAK_037      0x01
#define OAK_057      0x02
#define OAK_067      0x03
#define OAK_077      0x04
#define OAK_083      0x05
#define OAK_087      0x06
#define OAK_107      0x07
#define OAK_109      0x08
#define OAK_111      0x09
#define OAK_112      0x0A
#define OAK_117      0x0B

static char * oak_chip_name[] =
{
	"", "037C", "057", "067", "077", "083", "087", "64107",
	"64109", "64112", "64117"
};

unsigned int oak_chip, oak_mem;

char oak_test(void)
{
	unsigned int sub;

	if (testinx(0x3DE, 9) || testinx2(0x3DE, 0xD, 0x38))
	{
		if (testinx2(0x3DE, 0x23, 0x1F))
		{
			switch(rdinx(0x3DE, 2) & 6)
			{
				case 0: oak_mem = 256; break;
				case 2: oak_mem = 512; break;
				case 4: oak_mem = 1024; break;
				case 6: oak_mem = 2048; break;
			}
			if ((rdinx(0x3DE, 0) & 2) == 0)
				oak_chip = OAK_087;
			else
				oak_chip = OAK_083;
		}
		else
		{
			sub = inportb(0x3DE) >> 5;
			switch(sub)
			{
				case 0: oak_chip = OAK_037; break;
				case 2: oak_chip = OAK_067; break;
				case 5: oak_chip = OAK_077; break;
				case 7: oak_chip = OAK_057; break;
				default: oak_chip = OAK_UNKNOWN;
			}
			switch(rdinx(0x3DE, 0xD) >> 6)
			{
				case 2: oak_mem = 512; break;
				case 1:
				case 3: oak_mem = 1024; break;	// 1 might not give 1M??
			}
		}
        	return 1;
	}
	else
	if (testinx(0x3DE, 0xF0) && testinx(0x3DE, 0xF1))
	{
		sub = rdinx(0x3DE, 1)*256+rdinx(0x3DE, 0);
		switch(LOBYTE(sub))
		{
			case 3: oak_chip = OAK_107; break;
			case 4: oak_chip = ((sub >> 12) > 2) ? OAK_109 : OAK_107; break;
			case 5: oak_chip 	= OAK_109; break;
			case 6: oak_chip = ((sub >> 12) == 0) ? OAK_111 : OAK_112; break;
			case 7: oak_chip = OAK_117; break;
		}
		switch(rdinx(0x3DE, 2) & 14)
		{
			case 0: oak_mem = 256; break;
			case 2: oak_mem = 512; break;
			case 4: oak_mem = 1024; break;
			case 6: oak_mem = 2048; break;
			case 8: oak_mem = 4096; break;
		}
		modinx(0x3DE, 0x19, 0x3F, 0x31);
		return 1;
	}
	return 0;
}

unsigned int oak_chiptype(void)
{
	return oak_chip;
}

unsigned int oak_memory(void)
{
	return oak_mem;
}

char * oak_get_name(void)
{
 	if (oak_chip)
	{
	 	return oak_chip_name[oak_chip];
        }
	return "OAK Unknown";
}

void oak_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	if (oak_chip <= OAK_083)
		wrinx(0x3DE, 0x11, bank*17);
	else
	{
		wrinx(0x3DE, 0x23, bank);
		wrinx(0x3DE, 0x24, bank);
	}
}

GraphicDriver oak_driver =
{
	oak_chiptype,
	oak_get_name,
	oak_memory,
	NULL,
	NULL,
	oak_setbank
};

char Check_OAK(GraphicDriver * driver)
{
	*driver = oak_driver;
	return (oak_test());
}
