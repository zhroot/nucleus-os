#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define CPQ_UNKNOWN  0x000
#define CPQ_AVPORT   0x010
#define CPQ_IVGS     0x020
#define CPQ_AVGA     0x040
#define CPQ_QV       0x080	// The original QVision
#define CPQ_QV1024   0x100
#define CPQ_QV1280   0x200

static unsigned int compaq_chip, compaq_mem;

static char compaq_test(void)
{
	unsigned int old, sub;
	char result;

	result = 0;
	old = rdinx(GRA_I, 0xF);
	wrinx(GRA_I, 0xF, 0);
	if (! testinx(GRA_I, 0x45))
	{
		wrinx(GRA_I, 0xF, 5);
		if (testinx(GRA_I, 0x45))
		{
			result = 1;
			sub  = rdinx(GRA_I, 0xC) >> 3;
			switch(sub)
			{
				case 3: compaq_chip = CPQ_IVGS; break;
				case 5: compaq_chip = CPQ_AVGA; break;
				case 6: compaq_chip = CPQ_QV; break;
				case 0xE:
					compaq_chip = (rdinx(GRA_I, 0x56) & 4) ? CPQ_QV1280 : CPQ_QV1024;
					break;
				case 0x10: compaq_chip = CPQ_AVPORT; break;	// What is this ?
				default: compaq_chip = CPQ_UNKNOWN;
			}
			if ((rdinx(GRA_I, 0xC) & 0xB8) == 0x30)	// QVision
			{
				wrinx(GRA_I, 0xF, 5);
				switch(rdinx(GRA_I, 0x54))
				{
					case 0: compaq_mem = 1024; break;	// old QV1024 fix
					case 2: compaq_mem = 512; break;
					case 4: compaq_mem = 1024; break;
					case 8: compaq_mem = 2048; break;
				}
			}
                	else
				compaq_mem = 512;
		}
	}
	wrinx(GRA_I, 0xF, old);
	return result;
}

static unsigned int compaq_chiptype(void)
{
	return compaq_chip;
}

static unsigned int compaq_memory(void)
{
	return compaq_mem;
}

static char * compaq_get_name(void)
{
	switch(compaq_chip)
	{
		case CPQ_AVPORT: return "Compaq Adv VGA Port";
		case CPQ_IVGS  : return "Compaq IVGS";
		case CPQ_AVGA  : return "Compaq AVGA";
		case CPQ_QV    : return "Compaq QVision";
		case CPQ_QV1024: return "Compaq QVision 1024";
		case CPQ_QV1280: return "Compaq QVision 1280";
	}
	return "Compaq Unknown";
}

static void compaq_setbank(unsigned int bank)
{
	unsigned int x;
	if (current_bank == bank)
 		return;
	current_bank = bank;
	wrinx(GRA_I, 0xF, 5);
	bank <<= 1;
	if ((compaq_chip >= CPQ_QV1024) &&
		(inportb(0x23C7) & 0x10))
		x = 1;
	else
		x = 3;
	if ((compaq_chip == CPQ_AVGA) && (mem_mode == _pl4))
		x = 5;
	wrinx(GRA_I, 0x45, bank << x);
	if (rdinx(GRA_I, 0x40) & 1)
		bank++;
	wrinx(GRA_I, 0x46, bank << x);
}

GraphicDriver compaq_driver =
{
	compaq_chiptype,
	compaq_get_name,
	compaq_memory,
	NULL,
	NULL,
	compaq_setbank
};

char Check_Compaq(GraphicDriver * driver)
{
	*driver = compaq_driver;
	return (compaq_test());
}

