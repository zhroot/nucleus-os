#include <video/graphic.h>
#include <video/graph.h>
#include <support.h>

#define AHEAD_A 0x10
#define AHEAD_B 0x20

static unsigned int ahead_chip, ahead_mem;

static char ahead_test(void)
{
	unsigned int old;
	char result;

	result = 0;
	old = rdinx(GRA_I, 0xF);
	wrinx(GRA_I, 0xF, 0);
	if (! testinx2(GRA_I, 0xC, 0xFB))
	{
		wrinx(GRA_I, 0xF, 0x20);
		if (testinx2(GRA_I, 0xC, 0xFB))
		{
			result = 1;
			switch(rdinx(GRA_I, 0xF) & 15)
			{
				case 0: ahead_chip = AHEAD_A; break;
				case 1: ahead_chip = AHEAD_B; break;
			}
			switch(rdinx(GRA_I, 0x1F) & 3)
			{
				case 0: ahead_mem = 256; break;
				case 1: ahead_mem = 512; break;
				case 3: ahead_mem = 1024; break;
			}
		}
	}
	wrinx(GRA_I, 0xF, old);
	return result;
}

static unsigned int ahead_chiptype(void)
{
	return ahead_chip;
}

static char * ahead_get_name(void)
{
	switch(ahead_chip)
	{
		case AHEAD_A: return "Ahead V5000A";
		case AHEAD_B: return "Ahead V5000B";
	}
	return "Acer Unknown";
}

static unsigned int ahead_memory(void)
{
	return ahead_mem;
}

static void ahead_setbank(unsigned int bank)
{
	unsigned char x;
	if (current_bank == bank)
 		return;
	current_bank = bank;
	if (ahead_chip == AHEAD_B)
		wrinx(GRA_I, 13, bank*17);
	else
	{
		wrinx(GRA_I, 13, bank >> 1);
		x = inportb(0x3cc) & 0xdf;
		if (bank % 2)
			x += 32;
		outportb(0x3c2, x);
	}
}

GraphicDriver ahead_driver =
{
	ahead_chiptype,
	ahead_get_name,
	ahead_memory,
	NULL,
	NULL,
	ahead_setbank
};

char Check_Ahead(GraphicDriver * driver)
{
	*driver = ahead_driver;
	return (ahead_test());
}
