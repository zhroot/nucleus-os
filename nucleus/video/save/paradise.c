#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define WD_UNKNOWN  0x000
#define WD_PVGA1A   0x001
#define WD_90C00    0x002
#define WD_90C10    0x003
#define WD_90C11    0x004
#define WD_90C20    0x005
#define WD_90C20A   0x006
#define WD_90C22    0x007
#define WD_90C24    0x008
#define WD_90C26    0x009
#define WD_90C30    0x00A
#define WD_90C31    0x00B
#define WD_90C33    0x00C
#define WD_9710     0x00D

static char * paradise_chip_name[] =
{
	"PVGA1A", "90C00", "90C10", "90C11", "90C20", "90C20A", "90C22",
	"90C24", "90C26", "90C30", "90C31", "90C33","9710"
};

unsigned int paradise_chip, paradise_mem;

char paradise_test(void)
{
	unsigned int old, old2, sub;
	char result;

	result = 0;
	old = rdinx(GRA_I, 0xF);
	old2 = rdinx(CRT_I, 0x29);
	wrinx(GRA_I, 0xF, 5);	// Unlock them again
	modinx(CRT_I, 0x29, 0x8F, 0x85);	// Unlock WD90Cxx registers
	if (testinx2(GRA_I, 9, 0x7F)) //&& testinx(CRT_I, 0x2B))
	{
		if ((rdinx(CRT_I, 0x33) == 0x39) &&
			(rdinx(CRT_I, 0x34) == 0x37) &&
			(rdinx(CRT_I, 0x35) == 0x31) &&
 			(rdinx(CRT_I, 0x36) == 0x30))
		{
			result = 1;
			paradise_chip = WD_9710;
			outportw(0x23C0, 0xF016);
			outportw(0x23C0, 0xF0C1);
			switch(inportw(0x23C0) & 3)
			{
				case 0: paradise_mem = 1024; break;
				case 1: paradise_mem = 2048; break;
				case 2: paradise_mem = 4096; break;
			}
			wrinx(CRT_I, 0x29, old2);
		}
		else
		{
			setinx(GRA_I,0xF,0x17);	//Lock registers
			if (! testinx2(GRA_I, 9, 0x7F))
			{
				wrinx(GRA_I, 0xF, 5);	//Unlock them again
				if (testinx2(GRA_I, 9, 0x7F))
				{
				 	result = 1;
					old2 = rdinx(CRT_I, 0x29);
					modinx(CRT_I, 0x29, 0x8F, 0x85);	//Unlock WD90Cxx registers
					if (!testinx(CRT_I, 0x2B))
						paradise_chip = WD_PVGA1A;
					else
					{
						wrinx(SEQ_I, 6, 0x48);	//Enable C1x extensions
						if (! testinx2(SEQ_I, 7, 0xF0))
							paradise_chip = WD_90C00;
						else
						if (! testinx(SEQ_I, 0x10))
						{
							if (testinx2(CRT_I, 0x31, 0x68))
								paradise_chip = WD_90C22;
							else
							if (testinx2(CRT_I, 0x31, 0x90))
								paradise_chip = WD_90C20A;
							else
								paradise_chip = WD_90C20;
							wrinx(CRT_I, 0x34, 0xA6);
							if (rdinx(CRT_I, 0x32) & 0x20)
								wrinx(CRT_I, 0x34, 0);
						}
						else
						{
							if (testinx2(SEQ_I, 0x14, 0xF))
							{
								wrinx(CRT_I, 0x34, 0);	//Disable c2x registers
								wrinx(CRT_I, 0x35, 0);	//Disable c2x registers}
								sub = (rdinx(CRT_I,0x36) << 8)+rdinx(CRT_I,0x37);
								switch(sub)
								{
									case 0x3234: paradise_chip = WD_90C24; break;
									case 0x3236: paradise_chip = WD_90C26; break;
									case 0x3330: paradise_chip = WD_90C30; break;
									case 0x3331: paradise_chip = WD_90C31; break;
									case 0x3333: paradise_chip = WD_90C33; break;
								}
							}
							else
							if (! testinx2(SEQ_I, 0x10, 4))
								paradise_chip = WD_90C10;
							else
								paradise_chip = WD_90C11;
						}
					}
					wrinx(GRA_I, 0xF, 5);	//Unlock them again
					switch(rdinx(GRA_I, 0xB) >> 6)
					{
						case 2: paradise_mem = 512; break;
						case 3: paradise_mem = 1024; break;
					}
					if ((paradise_chip >= WD_90C33) &&
						(rdinx(CRT_I, 0x3E) & 0x80))
						paradise_mem = 2048;
					wrinx(CRT_I, 0x29, old2);
				}
			}
		}
	}
	wrinx(GRA_I, 0xF, old);
	return result;
}

char * paradise_get_name(void)
{
	if (paradise_chip)
	{
	 	return paradise_chip_name[paradise_chip];
	}
	return "Paradise/WD Unknown";
}

unsigned int paradise_memory(void)
{
	return paradise_mem;
}

unsigned int paradise_chiptype(void)
{
	return paradise_chip;
}

void paradise_setbank(unsigned int bank)
{
        if (current_bank == bank)
                return;
        current_bank = bank;
	if (paradise_chip >= WD_9710)
	{
		outportw(0x23C0, 0xF002);
		outportw(0x23C0, bank*16);
	}
	else
	{
		wrinx(GRA_I, 9, bank << 4);
		wrinx(GRA_I, 0xA, bank << 4);
		if (paradise_chip == WD_90C33)
			modinx(SEQ_I, 0x14, 0xC0, ((bank >> 4) & 1)*0xC0);
	}
}

GraphicDriver paradise_driver =
{
	paradise_chiptype,
	paradise_get_name,
	paradise_memory,
	NULL,
	NULL,
	paradise_setbank
};

char Check_Paradise(GraphicDriver * driver)
{
	*driver = paradise_driver;
	return (paradise_test());
}

