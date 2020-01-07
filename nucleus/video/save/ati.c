#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

#define ATI_UNKNOWN 0x000
#define ATI_EGA     0x005
#define ATI_18800   0x010
#define ATI_18800_1 0x020
#define ATI_28800_2 0x030
#define ATI_28800_4 0x040
#define ATI_28800_5 0x050
#define ATI_28800_6 0x060

unsigned int ati_chip, ati_mem, ati_ioaddr;
char * ati_crt;

static char ati_test(void)
{
 	unsigned int w, tmp1;
    unsigned char tmp2;
    char bios[100];

    getbios(bios, 0x31, 9);
  	if (bios == "761295520")
        {
		_dosmemgetw((0xC000 << 4) + 0x40, 1, &tmp1);
		switch(tmp1)
		{
			case 0x3F33: //264CT
			case 0x3133:
	             		{
					ati_ioaddr = 0x1CE;
					w = rdinx(ati_ioaddr, 0xBB);
					switch(w & 15)
	                                {
						case 0: ati_crt = "EGA"; break;
						case 1: ati_crt = "Analog Monochrome"; break;
						case 2: ati_crt = "Monochrome"; break;
						case 3: ati_crt = "Analog Color"; break;
						case 4: ati_crt = "CGA"; break;
						case 6: ati_crt = ""; break;
						case 7: ati_crt = "IBM 8514/A"; break;
						default: ati_crt = "Multisync";
					}
                                	_dosmemgetb((0xC000 << 4) + 0x43, 1, &tmp2);
					switch(tmp2)
                                        {
						case 0x31: ati_chip = ATI_18800; break;
						case 0x32: ati_chip = ATI_18800_1; break;
						case 0x33: ati_chip = ATI_28800_2; break;
						case 0x34: ati_chip = ATI_28800_4; break;
						case 0x35:
						case 0x36:
                                                     	if ((rdinx(ati_ioaddr, 0xAA) & 15) == 6)
								ati_chip = ATI_28800_6;
							else
								ati_chip = ATI_28800_5;
							break;
                                                default: ati_chip = ATI_UNKNOWN;
					}
					switch(ati_chip)
                                        {
						case ATI_18800:
						case ATI_18800_1:
        						if (rdinx(ati_ioaddr, 0xBB) & 0x20)
								ati_mem = 512;
							break;
						case ATI_28800_2:
                                                     	if (rdinx(ati_ioaddr, 0xB0) & 0x10)
								ati_mem = 512;
							break;
						case ATI_28800_4:
						case ATI_28800_5:
						case ATI_28800_6:
                                                	{
                                                      		switch(rdinx(ati_ioaddr, 0xB0) & 0x18)
                                                                {
									case 0x00: ati_mem = 256; break;
									case 0x10: ati_mem = 512; break;
									case 0x08:
									case 0x18: ati_mem = 1024; break;
								}
                                                        } break;
					}
				} break;
			case 0x3233: ati_chip = ATI_EGA; break;
		}
	        return 1;
	}
	return 0;
}

static unsigned int ati_chiptype(void)
{
	return ati_chip;
}

static unsigned int ati_memory(void)
{
	return ati_mem;
}

static char * ati_get_name(void)
{
	switch(ati_chip)
        {
		case ATI_EGA     : return "ATI EGA";
		case ATI_18800   : return "ATI VGA Wonder 18800";
		case ATI_18800_1 : return "ATI VGA Wonder (18800-1)";
		case ATI_28800_2 : return "ATI VGA Wonder (28800-2)";
		case ATI_28800_4 : return "ATI VGA Wonder (28800-4)";
		case ATI_28800_5 : return "ATI VGA Wonder XL (28800-5)";
		case ATI_28800_6 : return "ATI VGA Wonder XL (28800-6)";
	}
	return "ATI Unknown";
}

static void ati_setbank(unsigned int bank)
{
	unsigned int x;

	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (ati_chip == ATI_18800)
		modinx(ati_ioaddr, 0xB2, 0x1E, bank << 1);
	else
	{
		x = (bank & 15)*0x22;          //Roll bank nbr into bit 0
		wrinx(ati_ioaddr, 0xB2, HIBYTE(x) | LOBYTE(x));
	}
}

GraphicDriver ati_driver =
{
	ati_chiptype,
	ati_get_name,
	ati_memory,
	NULL,
	NULL,
	ati_setbank
};

char Check_Ati(GraphicDriver * driver)
{
	*driver = ati_driver;
	return (ati_test());
}
