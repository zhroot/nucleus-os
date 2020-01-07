#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>
#include <video/tseng.h>
#include <video/et6000.h>

unsigned int et6000_chip, et6000_mem;

static char et6000_test(void)
{
	unsigned char x;
	unsigned int ioaddr = 0;
	char result;

	result = 0;
	outportb(0x3BF, 3);
	outportb(CRT_I+4, 0xA0);	//Enable Tseng 4000 extensions
	if (tstrg(0x3CD, 0x3F))
	{
		if (testinx2(CRT_I, 0x33, 0xF))
		{
			if (tstrg(0x3CB,0x33))
			{
				if (rdinx(0x217A,0xEC) == 15)
				{
					result = 1;
					x  =rdinx(CRT_I, 0x21);
					switch(x)
					{
						case 0x00: ioaddr = 0xF100; break;
						case 0xFF: ioaddr = (rdinx(CRT_I, 0x22) == 0xFF) ? 0xF100 :  0xFF00; break;
						default: ioaddr = x << 8;
					}
				}
				if (inportb(ioaddr+3) == 0x47)	//PCI device ID
					et6000_chip = TSENG_ET6300;
				else
				if (inportb(ioaddr+8) >= 0x70)
					et6000_chip = TSENG_ET6100;
				else
					et6000_chip = TSENG_ET6000;
			}
			else
				et6000_chip = TSENG_UNKNOWN;
			if ((inportb(0x3C2) & 3) == 3)
			{
				et6000_mem = ((inportb(ioaddr+0x47) & 7)+1)*256;
				if (inportb(ioaddr+0x45) & 4)
					et6000_mem *= 2;
			}
		        else
			{
			 	switch(inportb(ioaddr+0x45) & 7)
				{
					case 0: et6000_mem = 1024; break;
					case 1: et6000_mem = 2048; break;
					case 2: et6000_mem = 4096; break;
					case 4: et6000_mem = 2048; break;
					case 5: et6000_mem = 4096; break;
					case 6: et6000_mem = 8192; break;
//					default: et6000_mem = check_mem(64, et6000_setbank);
				}
			}
		}
	}
	return result;
}

static unsigned int et6000_chiptype(void)
{
	return et6000_chip;
}

static unsigned int et6000_memory(void)
{
	return et6000_mem;
}

static char * et6000_get_name(void)
{
	switch(et6000_chip)
	{
		case TSENG_ET6000: return "Tseng ET6000";
		case TSENG_ET6100: return "Tseng ET6100";
		case TSENG_ET6300: return "Tseng ET6300";
	}
	return "Tseng ET600 Unknown";
}

static void et6000_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	outportb(0x3CD, (bank & 15)*17);
	outportb(0x3CB, (bank >> 4)*17);
}

GraphicDriver et6000_driver =
{
	et6000_chiptype,
	et6000_get_name,
	et6000_memory,
	NULL,
	NULL,
	et6000_setbank
};

char Check_ET6000(GraphicDriver *driver)
{
	if (et6000_test())
	{
	 	*driver = et6000_driver;
		return 1;
	}
	return 0;
}
