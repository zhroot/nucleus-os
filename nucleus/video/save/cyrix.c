#include <support.h>
#include <video/graphic.h>
#include <video/graph.h>

unsigned int cyrix_chip, cyrix_mem;

static char cyrix_test(void)
{
	unsigned int x, y = 0;
    char result;

    result = 0;
	x = rdinx(CRT_I, 0x30);
	wrinx(CRT_I, 0x30, 0);	// Disable extension
	if (rdinx(CRT_I, 0x30) == 255)
	{
		wrinx(CRT_I, 0x30, 0x57);
		wrinx(CRT_I, 0x30, 0x4C);	// Enable extension
		if (! rdinx(CRT_I, 0x30))
		{
                 	result = 1;
			cyrix_mem = 64*rdinx(CRT_I, 0x3E);
		}
	}
	wrinx(CRT_I, 0x47, y);
	wrinx(CRT_I, 0x30, x);
    return result;
}

static unsigned int cyrix_chiptype(void)
{
	return cyrix_chip;
}

static unsigned int cyrix_memory(void)
{
	return cyrix_mem;
}

static char * cyrix_get_name(void)
{
	return "Cyrix Unknown";
}

static void cyrix_setbank(unsigned int bank)
{
	if (current_bank == bank)
 		return;
	current_bank = bank;
	wrinx(CRT_I, 0x47, bank);
}

GraphicDriver cyrix_driver =
{
	cyrix_chiptype,
	cyrix_get_name,
	cyrix_memory,
	NULL,
	NULL,
	cyrix_setbank
};

char Check_Cyrix(GraphicDriver * driver)
{
	*driver = cyrix_driver;
	return (cyrix_test());
}

